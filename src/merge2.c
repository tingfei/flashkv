#include "segtable.h"
#include "flash.h" //for device
#include "tools.h"

#include <stdio.h>
#include <math.h>  //pow

extern struct ATABLE *active_table;
extern struct LEV0 * lev0_head;
extern char *table_finder_0;
extern char *lev0_tables[];
extern char *levels_summary;

extern struct FINDER_ENTRY *tip_tables_entry[MAX_LEV]; 
extern struct FINDER_ENTRY *first_tables_entry[MAX_LEV];

extern uint64_t serials_width[];
extern char *seg_bit_maps[];



int give_tip_table(char **tip_table, int full_lev, char **tip_first_key, char **tip_last_key);
int give_crossed_serials(int, char *, char *, uint64_t*, struct FINDER_ENTRY **);
int fill_crossed_tables(char ** crossed_tables, uint64_t *crossed_serials, int crossed_num);
int fill_big_table2(char *big_table,char *tip_table, char** crossed_tables, int crossed_num);
int split_big_table2(char * big_table, int crossed_num, struct FINDER_ENTRY *insert_point, int lev);
int chop_lev(int full_lev);

int merge2_num=0;

int merge2(int full_lev){
	//exit(1);
	merge2_num++;
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>I am merge2, full_lev=%d, merge2_num=%d \n", full_lev,merge2_num);
	//printf("I am merge %d, levels_summary=%p\n",merge2_num, levels_summary);
	if(full_lev==5){
		printf("merge2 puase, full_lev comes to=%d\n",full_lev);
		exit(1);
	}
	int i;
	for(i=0;i<LEV0_NUM;i++){
		//printf("lev0_tbales[%d]=%p\n",i, lev0_tables[i]);
	}
	
	//print_table("lev0_tables[0]",lev0_tables[0]);
	//print_table("lev0_tables[1]",lev0_tables[1]);
	//print_table("lev0_tables[2]",lev0_tables[2]);
	//print_table("lev0_tables[3]",lev0_tables[3]);
	//print_table("lev0_tables[4]",lev0_tables[4]);
	
	printf("merge2, the full level is  %d\n",full_lev);
	char *tip_table=NULL;
	char *tip_first_key;
	char *tip_last_key;
	give_tip_table(&tip_table, full_lev,&tip_first_key, &tip_last_key );
	//printf("tip table\n");
	//print_table("tip table", tip_table);
	//printf("tip_first_key=%s, tip_last_key=%s\n", tip_first_key,tip_last_key );
	
	uint64_t crossed_serials[LEV_PUFFER*LEV_PUFFER]={0};//this should be enough
	struct FINDER_ENTRY *insert_point=NULL;
//printf("222222222, full_lev=%d\n",full_lev);		
	//printf("before give_crossed_serials, insert_point=%p\n",insert_point);
	int crossed_num=give_crossed_serials(full_lev+1, tip_first_key, tip_last_key, crossed_serials, &insert_point);
	//printf("after give_crossed_serials, crossed_num=%d, insert_point=%p\n",crossed_num,insert_point);
//printf("5555555555555, full_lev=%d\n",full_lev);		
	char *crossed_tables[crossed_num];
	fill_crossed_tables( crossed_tables, crossed_serials, crossed_num);
	
	
	//printf("crossed table\n");
	for(i=0;i<crossed_num;i++){
		//printf("%d\n",i);
		//print_table("crossed table",crossed_tables[i] );
	}
	
	int big_table_size=(crossed_num+2)* test_seg_bytes;//crossed_num may be -1
	char *big_table=(char*)malloc(big_table_size);
	memset(big_table,0, big_table_size );
	fill_big_table2(big_table,tip_table, crossed_tables,crossed_num );
	//print_table("tip_table",tip_table);
	//print_table("big_table",big_table);
	
//printf("000000000, full_lev=%d\n",full_lev);		
	split_big_table2(big_table, crossed_num,  insert_point, full_lev+1);
	
//printf("aaaaaaaaaaa, full_lev=%d\n",full_lev);	
	chop_lev(full_lev);//after merge, the tip table should be chopped off
	
	//printf("merge2_num=%d, first_tables_entry[1]->next->serial_num=%d\n", merge2_num,first_tables_entry[1]->next->serial_num);
	i=0;
	
	struct FINDER_ENTRY *f= first_tables_entry[1]->next;
	while(f!=NULL){
			i++;
			printf("%d: serial=%d, first_key=%s, last_key=%s\n",i,f->serial_num,f->first_key,f->last_key);
			f=f->next;
		
	}
	
	 //print_table("serial 10", read_seg(10));
	print_bit_map("bit map 1 after merge2 finised",seg_bit_maps[1],serials_width[1]);
	//print_bit_map("bit map 2",seg_bit_maps[2],serials_width[2]);
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>I am merge2, end, full_lev=%d, merge2_num=%d \n", full_lev,merge2_num);
	if(merge2_num==6000){
		//print_table("lev0-2", lev0_tables[1]);
		exit(1);
	}
	return 0;
	
}





int give_tip_table(char **tip_table, int full_lev,char **tip_first_key, char **tip_last_key){
	print_table("in give_tip_table", *tip_table);
	
	if(full_lev==0){
		*tip_table=lev0_tables[LEV0_NUM-1];
		*tip_first_key=lev0_tables[LEV0_NUM-1];
		*tip_last_key=table_finder_0+ FINDER_ENTRY_LENGTH*(LEV0_NUM-1) + FINDER_KEY_LENGTH;
		
		return;
	}
	
	//now the lev full_lev must be full and the lev number is at least 1, we use tip_tables_entry[i] to get the table
	//printf("tip_tables_entry[full_lev]->serial_num=%d\n",tip_tables_entry[full_lev]->serial_num);
	*tip_table=read_seg(tip_tables_entry[full_lev]->serial_num);
	*tip_first_key=tip_tables_entry[full_lev]->first_key;
	*tip_last_key=tip_tables_entry[full_lev]->last_key;
	//printf("in give_tip_table, *tip_table=%p\n",*tip_table);
	//print_table("in giv_tip_table, tip_table",*tip_table);
	
}





int give_crossed_serials(int lev, char *tip_first_key, char *tip_last_key, uint64_t *crossed_serials, struct FINDER_ENTRY  **insert_point){
	struct FINDER_ENTRY *finder=first_tables_entry[lev]->next;//point to the header
	*insert_point=first_tables_entry[lev];//point to the header node
	//printf("give_crossed_serials, finder=%p,lev=%d\n",finder,lev);
	int total_crossed=0;
	
	
	while(finder!=NULL){
//printf("xxxxxxxxxxxx,tip_last_key=%s ,finder->first_key=%s\n",tip_last_key,finder->first_key);
		if(strcmp(tip_last_key, finder->first_key)<0){//no following tables
			if(total_crossed==0){
				*insert_point=finder->pre;//this node has pre
				return 0;
			}
			return total_crossed;
		}
//printf("yyyyyyyyyyy\n");
		
		if(strcmp(tip_first_key, finder->last_key)>0){//no cross, but followings may be crossing
			*insert_point=finder;
			finder=finder->next;
			
			continue;
		}
//printf("zzzzzzzzzzz\n");
		
		//now that comes here, this table must be crossing
		if(total_crossed==0) *insert_point=finder->pre;//
		crossed_serials[total_crossed++]=finder->serial_num;
		finder=finder->next;
	
	}
//printf("give_crossed_serials, end\n");	
	return total_crossed;
	

}





int fill_crossed_tables(char ** crossed_tables, uint64_t *crossed_serials, int crossed_num){
	int i;
	for(i=0;i<crossed_num;i++){
		crossed_tables[i]=read_seg(crossed_serials[i]);
	
	}
	
	//if (merge2_num==2) exit(1);

}






int fill_big_table2(char *big_table,char *tip_table,  char** crossed_tables,int crossed_num){
	int i;
	char *sorted_active_table=tip_table;
	if(crossed_num<=0){//directly trans
		memcpy(big_table, sorted_active_table, test_seg_bytes);
		return 1;
	}
	char *union_crossed_tables=malloc( (crossed_num)* test_seg_bytes +1 );//+1 is for the case crossed_num=0; but 
	memset(union_crossed_tables, 0, (crossed_num)* test_seg_bytes +1);
	
	char *union_advancer=union_crossed_tables;//anvances the cross union
	
	for(i=0;i<crossed_num;i++){//copy the crossed tables to union_crossed_tables
			//uint32_t copy_size=*(uint32_t *) (crossed_tables[i] + test_seg_bytes - TABLE_END_RESERVED + TABLE_END_ZERO);
			
			char *advancer=crossed_tables[i];//every cycle this points to a next crossed table
			uint32_t copy_size=0;
			//printf("debug1, i=%d, advancer=%p\n",i,advancer);
			
			while(*advancer!=0){//caculate the i-th table size
				int kv_len=strlen(advancer)+1 + strlen(advancer+ strlen(advancer)+1 ) +1;
				copy_size+=kv_len;
				advancer+=kv_len;
				
			}
			memcpy(union_advancer,crossed_tables[i], copy_size );//copy the i-th crossed table
			union_advancer+= copy_size;
	}
	//print_table("union_crossed_tables in merging",union_crossed_tables);
	
	char *sorted_table_advancer=sorted_active_table;
	union_advancer=union_crossed_tables;
	char *big_table_advancer=big_table;
	
	i=0;
	while(1){
	
			if(*sorted_table_advancer==0){
			//printf("*sorted_table_advancer==0\n");
				while(*union_advancer!=0){
					int kv_len=strlen(union_advancer)+1 + strlen(union_advancer+ strlen(union_advancer)+1 ) +1;
					memcpy(big_table_advancer, union_advancer,kv_len );
					union_advancer += kv_len;
					big_table_advancer += kv_len;
				}
				break;
			}
			if(*union_advancer==0){
				//printf("*union_advancer==0\n");
				while(*sorted_table_advancer!=0){
					int kv_len=strlen(sorted_table_advancer)+1 + strlen(sorted_table_advancer+ strlen(sorted_table_advancer)+1 ) +1;
					memcpy(big_table_advancer, sorted_table_advancer,kv_len );
					sorted_table_advancer += kv_len;
					big_table_advancer += kv_len;
				}
				break;
			}
			
			int res=strcmp(sorted_table_advancer,union_advancer);
			if(res<=0){
				
				int kv_len=strlen(sorted_table_advancer)+1 + strlen(sorted_table_advancer+ strlen(sorted_table_advancer)+1 ) +1;
				//printf("res<=0, res=%d, kv_len=%d\n",res, kv_len);
				memcpy(big_table_advancer, sorted_table_advancer,kv_len );
				sorted_table_advancer += kv_len;
				big_table_advancer += kv_len;
				if(res==0){
					//printf("res=0, res=%d, kv_len=%d\n",res, kv_len);
					union_advancer += strlen(union_advancer) +1;//key
					union_advancer += strlen(union_advancer) +1;//value
				}
			}
			else{
				i++;
				//if(i>22) break;
				int kv_len=strlen(union_advancer)+1 + strlen(union_advancer+ strlen(union_advancer)+1 ) +1;
				//printf("res>0, res=%d, kv_len=%d, union_advancer=%s\n",res, kv_len,union_advancer);
				memcpy(big_table_advancer, union_advancer,kv_len );
				union_advancer += kv_len;
				big_table_advancer += kv_len;
			}
			
			
	}
	
	
	free(union_crossed_tables);
	
	//print_table("in fill big table, big table", big_table);
	
	//if(merge2_num==2) exit(1);

	return 1;

}













int split_big_table2(char * big_table, int crossed_num, struct FINDER_ENTRY *insert_point, int lev){
int i;

	//print_table("in splitting, big table", big_table);
	
	//if(merge2_num==2) exit(1);
//printf("1111111111 lev=%d\n",lev);	
	char *big_table_advancer=big_table;
	int splitting_counter=0;
	char *splitted_tables_pointer[crossed_num + 2]; //crossed_num can be -1, but an array's size should be at lesast 1
	int splitted_tables_num=0;
	
	char *manua_splitted_first_key[crossed_num+2];
	manua_splitted_first_key[0]=big_table_advancer;
	char *manua_splitted_last_key[crossed_num+2];
	char *start_pointer=big_table_advancer;//records the start point when splitting table
	//printf("0000 start_pointer=%p \n",start_pointer);
	char *p0,*p1;//for recording two
	//printf("1111 start_pointer=%p \n",start_pointer);
	//print_table("in splitted, big table",big_table);
	int flag=0;
	while(1){//every cycle advance a KV
		if(*big_table_advancer==0){//advance to the end, finish
				break;
		}
		p0=p1;//if would split, p0 points to the last key
		p1=big_table_advancer;//if would split, p1 points to the first key of the next table
		//printf("in while, p1=%s\n",p1);
		int len_key= strlen(big_table_advancer)+1;
		big_table_advancer += len_key;//advances the key
		int len_value =strlen(big_table_advancer)+1;
		big_table_advancer += len_value; //advances the value 		
		if(flag==1){
			//printf("splitted_tables_num=%d\n",splitted_tables_num);
			//printf("in splitting,manua_splitted_last_key[0]=%s\n",manua_splitted_last_key[0]);
		}
		if( splitting_counter + len_key + len_value + TABLE_END_RESERVED > test_seg_bytes){//splitting
			
			//printf("in splitting,splitted_tables_num=%d , p0=%s\n",splitted_tables_num,p0);
			splitted_tables_pointer[splitted_tables_num]=(char *)malloc(test_seg_bytes);
			memset(splitted_tables_pointer[splitted_tables_num],0 , test_seg_bytes);			
			memcpy(splitted_tables_pointer[splitted_tables_num], start_pointer, p1-start_pointer);
			
			
			manua_splitted_last_key[splitted_tables_num]=p0;
			//printf("in splitting,splitted_tables_num=%d , p0=%s, manua_splitted_last_key[%d]=%s \n",splitted_tables_num,p0,splitted_tables_num, manua_splitted_last_key[splitted_tables_num]);
			splitted_tables_num++;
			splitting_counter=0;				
			manua_splitted_first_key[splitted_tables_num]=p1;
			start_pointer=p1;//resetg the start pointer
			flag=1;
				
		}
		splitting_counter += len_key+len_value;
	}
	
	for(i=0;i<splitted_tables_num;i++){
		//printf("splitted_table:%d, first_key=%s, last_key=%s\n",i,manua_splitted_first_key[i],manua_splitted_last_key[i]);
		//print_table("splitted_table",splitted_tables_pointer[i]);
	}
	//but the last splitted tables has not been stored. we do this int the next
	//printf("4444 start_pointer=%p \n",start_pointer);//this will couse it to wrong address
	//printf("start_pointer=%s, p1=%s,test_seg_bytes=%d\n",start_pointer,p1,test_seg_bytes);
	
	manua_splitted_last_key[splitted_tables_num]=p1;
	splitted_tables_pointer[splitted_tables_num]=(char *)malloc(test_seg_bytes);
	memset(splitted_tables_pointer[splitted_tables_num], 0 , test_seg_bytes);
	//printf("p1=%p \n",p1);
	//printf("splitted_tables_pointer[splitted_tables_num]=%p, start_pointer=%p, big_table=%p\n",splitted_tables_pointer[splitted_tables_num],start_pointer,big_table);
	memcpy(splitted_tables_pointer[splitted_tables_num], start_pointer, big_table_advancer-start_pointer);
	
	//print_table("splitted_tables_pointer[0]",splitted_tables_pointer[0]);
	splitted_tables_num++;//make it the ture num of splitted tables
	//splitting finished. splitted tables are store int splitted_tables_pointer[]. there are splitted_tables_num splitted tables
	
	printf("crossed_num=%d,splitted_tables_num=%d, big_table_advancer-start_pointer=%d\n",crossed_num,splitted_tables_num,big_table_advancer-start_pointer);
	for(i=0;i<splitted_tables_num;i++){
		//printf("splitted_table:%d, first_key=%s, last_key=%s\n",i,manua_splitted_first_key[i],manua_splitted_last_key[i]);
		//print_table("splitted_table",splitted_tables_pointer[i]);
	}
	//garbage reclaim --begin
	//struct FINDER_ENTRY *insert_point_pre=insert_point->pre;
	
	if(insert_point!=NULL){//if insert_point is null, crossed_num must be less than 0

		struct FINDER_ENTRY *cross_tables_advancer;
		cross_tables_advancer=insert_point->next;
		struct FINDER_ENTRY *crossed_mark=cross_tables_advancer;
		for(i=0;i<crossed_num;i++){	
			cross_tables_advancer=cross_tables_advancer->next;
		}
		insert_point->next=cross_tables_advancer;//discard the crossed entry node
		if(cross_tables_advancer!=NULL) cross_tables_advancer->pre=insert_point;
		
		for(i=0;i<crossed_num;i++){		//free crossed entries and the respond segments
				struct FINDER_ENTRY *temp=crossed_mark;
				discard_seg(temp->serial_num);
				clear_bit_map(lev, temp->serial_num);
				crossed_mark=crossed_mark->next;
				free(temp);
				//free(lev0_tables[base_entry+i]);
				//lev0_tables[base_entry+i]=NULL;
		}
	}
	//garbage reclaim --end
	//printf("crossed_num",crossed_num,);
	//allocate new serials --begin
	uint64_t *new_serials;
	new_serials=allocate_serial(lev,splitted_tables_num);
	//assert
	//allocate new serials --end
	
	//new tables written --begin
	for(i=0;i<splitted_tables_num;i++){
		write_seg(splitted_tables_pointer[i], new_serials[i] );	
	}
	//new tables written --end

	//update finder entry list --begin
		//case 1: no crossing and insert point is null. crossing is 0
	
		//case 2: no crossing and insert point is not null. crossing is -1
		
		//case 3: crossing is positive 

		struct FINDER_ENTRY *new_entry;
		for(i=0;i<splitted_tables_num;i++  ){
			new_entry=(struct FINDER_ENTRY *)malloc(sizeof(struct FINDER_ENTRY ) );
			memset(new_entry,0, sizeof(struct FINDER_ENTRY ));
				//printf("splitted %d: first:%s  last:%s\n",i, manua_splitted_first_key[i],manua_splitted_last_key[i] );
			memcpy(new_entry->first_key, manua_splitted_first_key[i], strlen(manua_splitted_first_key[i]) );// make sure manua_splitted_first_key
										//is zeroed when created
			memcpy(new_entry->last_key, manua_splitted_last_key[i], strlen(manua_splitted_last_key[i]) );
			new_entry->serial_num=new_serials[i];
			
			new_entry->next=insert_point->next;
			insert_point->next=new_entry;
			new_entry->pre=insert_point;
			if(new_entry->next!=NULL){//insert to the tail
				new_entry->next->pre=new_entry;
			}
			else{
				tip_tables_entry[lev]=new_entry;
			}
			insert_point=new_entry;//advance the insert point
	
		}
//printf("22222222222222222222\n");
	//update finder entry list --end
	
	//other updates --begin
	int inc=0;
	if(crossed_num==0){
		inc=splitted_tables_num;
	}
	else{
		inc =splitted_tables_num-crossed_num;
	}
//printf("55555555555, levels_summary=%p, inc=%d, lev=%d \n",levels_summary,inc, lev);

	( *(int*) ( levels_summary+ (lev)*LEVELS_SUMMARY_ENTRY) )+= inc;
	
//printf("999999999999 lev=%d\n",lev);	


	//other updates --end
}








int chop_lev(int full_lev){
	printf("I am chop_lev, full_lev=%d\n",full_lev);

	if(full_lev==0){
	printf("ddddddddddd\n");
		memset(table_finder_0+ FINDER_ENTRY_LENGTH*(LEV0_NUM-1), 0, FINDER_ENTRY_LENGTH);//sweep the last entry
		//free the table
		free(lev0_tables[LEV0_NUM-1]);
		lev0_tables[LEV0_NUM-1]=NULL;
		//update the levels_summary ftl file
		
	
	}
	else{
	printf("gggggggggggg\n");
		//discard the segment
		struct FINDER_ENTRY *temp=tip_tables_entry[full_lev];
		tip_tables_entry[full_lev]=tip_tables_entry[full_lev]->pre;
		
		discard_seg(temp->serial_num);
		clear_bit_map(full_lev,temp->serial_num);
		//update the finder entry
		temp->pre->next=NULL;
		//free tip finder entry
		free(temp);
		temp=NULL;	
printf("jjjjjjjjjjjjj\n");		
	}
	
	( *(int*) ( levels_summary+ full_lev*LEVELS_SUMMARY_ENTRY) )--;

}



/*
			if(insert_point==NULL){// insert at the beginning
	

				first_tables_entry[lev]=new_entry;
				new_entry->next=NULL;
				new_entry->pre=NULL;
			}
			else{				
				struct FINDER_ENTRY * temp=insert_point->next;
				insert_point->next=new_entry;
				new_entry->next=temp;
				new_entry->pre=insert_point;
				insert_point=new_entry;//advance
			}
	*/