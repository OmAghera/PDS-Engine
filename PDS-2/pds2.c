#include "pds2.h"

struct DBInfo db_info = {
	.num_table = 0,
	.db_status = DB_CLOSE
};

struct TableInfo* get_tableInfo(char* table_name){
	char table[50];
	strcpy(table,table_name);
	strcat(table,".dat");
	for(int idx = 0;idx<db_info.num_table;idx++){
		if(strcmp(db_info.tinfo[idx].tname, table)==0){
			return &db_info.tinfo[idx];
		}
	}
	return NULL;
}
int create_db(char *table1_name,char *table2_name){
	int is_success1 =  create_table(table1_name);  
	int is_success2 =  create_table(table2_name);
	if(is_success1 == SUCCESS && is_success2 == SUCCESS){
		return SUCCESS;
	}
	else{
		return FAILURE;
	}
}
int create_table(char*table_name){
	char dbfilename[50];
	char ndxfilename[50];
	strcpy(dbfilename,table_name);
	strcpy(ndxfilename,table_name);
	strcat(dbfilename,".dat");
	strcat(ndxfilename,".ndx");
	FILE *fptr = fopen(dbfilename, "wb");
	FILE *ndxfile = fopen(ndxfilename,"wb"); 
	if(fptr == NULL || ndxfile == NULL) {return FAILURE;}
	else {
		int initailize_file = 0;
		fwrite(&initailize_file,sizeof(int),1,ndxfile);
		fclose(fptr);
		fclose(ndxfile);
		printf("file create success\n");
		return SUCCESS;
	}
}
int open_db(char *table1_name,char* table2_name,int rec1_size,int rec2_size){
	int is_success1 = open_table(table1_name,rec1_size);
	int is_success2 = open_table(table2_name,rec2_size);
	if(is_success1 == SUCCESS && is_success2 == SUCCESS){
		db_info.db_status = DB_OPEN;
		return SUCCESS;
	}
	else{
		return FAILURE;
	}

}
int open_table(char *table_name,int rec_size){
	if(db_info.num_table==2)return FAILURE;
	char dbfilename[50];
	char ndxfilename[50];
	strcpy(dbfilename,table_name);
	strcpy(ndxfilename,table_name);
	strcat(dbfilename,".dat");
	strcat(ndxfilename,".ndx");
	int this_table_idx = db_info.num_table;
	db_info.num_table++;
	db_info.tinfo[this_table_idx].tFile = fopen(dbfilename,"rb+");
	db_info.tinfo[this_table_idx].ndxFile = fopen(ndxfilename,"rb+");
	if(db_info.tinfo[this_table_idx].tFile == NULL || db_info.tinfo[this_table_idx].ndxFile == NULL) {
		db_info.num_table = 0;
		db_info.db_status = DB_CLOSE;
		return FAILURE;
	}
    else {
		strcpy(db_info.tinfo[this_table_idx].tname,dbfilename);
		strcpy(db_info.tinfo[this_table_idx].ndx_name,ndxfilename);
		db_info.tinfo[this_table_idx].rec_size = rec_size;
		struct Table_ndx temp;
		int rec = 0;
		fread(&rec,sizeof(int),1,db_info.tinfo[this_table_idx].ndxFile);
		fread(db_info.tinfo[this_table_idx].ndxArray,sizeof(struct Table_ndx),rec,db_info.tinfo[this_table_idx].ndxFile);
		db_info.tinfo[this_table_idx].rec_count = rec;
		db_info.tinfo[this_table_idx].table_status = DB_OPEN;
		fclose(db_info.tinfo[this_table_idx].ndxFile);
		printf("file open success\n");
        return SUCCESS;
	}
}

int store_table(int key,void* c,char* table_name){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	struct TableInfo* table = get_tableInfo(table_name);
	if(table == NULL) return FAILURE;
	if(table->table_status == DB_CLOSE) return FAILURE;
	fseek(table->tFile,0,SEEK_END);
	struct Table_ndx temp = {key,ftell(table->tFile),false,-1};
	table->ndxArray[table->rec_count++] = temp;
	fwrite(&key,sizeof(int),1,table->tFile);
	fwrite(c,table->rec_size,1,table->tFile);
	printf("store success\n");
	return SUCCESS;
}

int get_table(int key,void* coutput,char* table_name){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	struct TableInfo* table = get_tableInfo(table_name);
	if(table == NULL) return FAILURE;
	if(table->table_status == DB_CLOSE) return FAILURE;
	int location = -1;
	for(int idx = 0;idx<table->rec_count;idx++){
		if(table->ndxArray[idx].key == key){
			location = table->ndxArray[idx].loc;
		}
	}
	if(location == -1) return FAILURE;
	fseek(table->tFile,location+sizeof(int),SEEK_SET);
	fread(coutput,table->rec_size,1,table->tFile);
	return SUCCESS;
}
int update_table(int key,void *new,char* table_name){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	struct TableInfo* table = get_tableInfo(table_name);
	if(table == NULL) return FAILURE;
	if(table->table_status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<table->rec_count;idx++){
		if(table->ndxArray[idx].key == key){
			fseek(table->tFile,table->ndxArray[idx].loc + sizeof(int),SEEK_SET);
			fwrite(new,table->rec_size,1,table->tFile);
			return SUCCESS;
		}
	}
	return FAILURE;
}
int delete_table(int key,char* table_name){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	struct TableInfo* table = get_tableInfo(table_name);
	if(table == NULL) return FAILURE;
	if(table->table_status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<table->rec_count;idx++){
		if(table->ndxArray[idx].key == key){
			table->ndxArray[idx].old_key = key;
			table->ndxArray[idx].key = -1;
			table->ndxArray[idx].is_deleted = true;
			return SUCCESS;
		}
	}
	return FAILURE;
}
int undelete_table(int old_key,char* table_name){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	struct TableInfo* table = get_tableInfo(table_name);
	if(table == NULL) return FAILURE;
	if(table->table_status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<table->rec_count;idx++){
		if(table->ndxArray[idx].is_deleted == true && table->ndxArray[idx].old_key == old_key){
			table->ndxArray[idx].is_deleted = false;
			table->ndxArray[idx].key = old_key;
			table->ndxArray[idx].old_key = -1;
			return SUCCESS;
		}
	}
	return FAILURE;
}
int close_table(struct TableInfo* table){
	if(table->table_status == DB_CLOSE) return FAILURE;
	table->ndxFile = fopen(table->ndx_name,"wb");
	//fseek(db_info.ndx_file,0,SEEK_SET);
	if(table->ndxFile == NULL){
		return FAILURE;
	}
	fwrite(&table->rec_count,sizeof(int),1,table->ndxFile);
	fwrite(table->ndxArray,sizeof(struct Table_ndx),table->rec_count,table->ndxFile);
	fclose(table->tFile);
	fclose(table->ndxFile);
	printf("files closed success\n");
	table->table_status = DB_CLOSE;
	return SUCCESS;
}

int close_db(){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<db_info.num_table;idx++){
		int is__success = close_table(&db_info.tinfo[idx]);
		if(is__success == FAILURE)return FAILURE;
	}
	db_info.num_table=0;
	db_info.db_status = DB_CLOSE;
	return SUCCESS;
}