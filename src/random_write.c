#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "main.h"
#include "flash.h"
#include "segtable.h"
#include "tools.h"

#include <time.h>
extern char *levels_summary;

extern struct DEVICE device;
extern struct FINDER_ENTRY *first_tables_entry[MAX_LEV];
extern struct FINDER_ENTRY *tip_tables_entry[MAX_LEV];

extern int  merge_recur_num;
extern uint64_t put_counter;

void generate_string(char *value, int max);

int random_write(){
	printf("i am random_write\n");	
	
	
	uint64_t i;
	int end=0;
	uint64_t size=0;
	size=10000000;
	printf("Input size:\n");
	scanf("%llu",&size);
	
	srand( (unsigned)time( NULL ) );
	for(put_counter=0;put_counter<size;put_counter++){
		char *key=malloc(10);
		memset(key,0,10);
		char *value=malloc(20);
		memset(key,0,10);
		
		sprintf(key,"%019llu",rand());
		
		generate_string(value, 20);
		
		//printf("key=%s, value=%s\n",key,value);
		put(key,value);
	}
	
	//int lev0_nums=0;
	
	
	//printf("lev1 tip entry %d: serial=%d, first_key=%s | last_key=%s\n",1, tip_tables_entry[1]->serial_num , tip_tables_entry[1]->first_key,  tip_tables_entry[1]->last_key);	
	
}