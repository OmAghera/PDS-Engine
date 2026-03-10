#include "pds4.h"

struct DBInfo db_info = {
	.num_table = 0,
	.rel_count = 0,
	.db_status = DB_CLOSE,
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
int create_db(char*db_name){
	char dbname[50];
	strcpy(dbname,db_name);
	strcat(dbname,".sch");
	FILE *fptr = fopen(dbname,"rb");
	if(fptr == NULL){
		struct DBInfo db;
		fptr = fopen(dbname,"wb");
		strcpy(db.dbname,dbname);
		db.num_table = 0;
		db.rel_count = 0;
		db.db_status = DB_CLOSE;
		fwrite(&db,sizeof(struct DBInfo),1,fptr);
		fclose(fptr);
		return SUCCESS;
	}
	else{
		fclose(fptr);
		return FAILURE;
	}
}
int create_table(char*table_name,int rec_size){
	if(db_info.db_status == DB_CLOSE) return FAILURE;
	char dbfilename[100];
	char ndxfilename[100];
	strcpy(dbfilename,table_name);
	strcpy(ndxfilename,table_name);
	strcat(dbfilename,".dat");
	strcat(ndxfilename,".ndx");
	FILE *fptr = fopen(dbfilename, "wb");
	FILE *ndxfile = fopen(ndxfilename,"wb"); 
	if(fptr == NULL || ndxfile == NULL) {
		fclose(fptr);
		fclose(ndxfile);
		return FAILURE;
	}
	else {
		int initailize_file = 0;
		fwrite(&initailize_file,sizeof(int),1,ndxfile);
		fclose(fptr);
		fclose(ndxfile);
		strcpy(db_info.tinfo[db_info.num_table].tname,dbfilename);
		strcpy(db_info.tinfo[db_info.num_table].ndx_name,ndxfilename);
		db_info.tinfo[db_info.num_table].rec_size = rec_size;
		db_info.tinfo[db_info.num_table].rec_count = 0;
		db_info.tinfo[db_info.num_table].table_status = DB_CLOSE;
		int status = open_table(&db_info.tinfo[db_info.num_table]);
		if(status == SUCCESS)db_info.num_table++;
		return status;
	}
}
int open_db(char *dbname){
	if(db_info.db_status == DB_OPEN)return FAILURE;
	char dbfilename[100];
	strcpy(dbfilename,dbname);
	strcat(dbfilename,".sch");
	FILE *dbptr = fopen(dbfilename,"rb+");
	if(dbptr == NULL) return FAILURE;
	fread(&db_info,sizeof(struct DBInfo),1,dbptr);
	db_info.db_status = DB_OPEN;
	for(int idx= 0;idx<db_info.num_table;idx++){
		int status = open_table(&db_info.tinfo[idx]);
		if(status == FAILURE){
			close_db();
			return FAILURE;
		}
	}
	for(int idx = 0;idx<db_info.rel_count;idx++){
		int status = open_relation(&db_info.rInfo[idx]);
		if(status == FAILURE){
			close_db();
			return FAILURE;
		}
	}
	return SUCCESS;
}
int open_table(struct TableInfo *table){
	table->tFile = fopen(table->tname,"rb+");
	table->ndxFile = fopen(table->ndx_name,"rb+");
	if(table->tFile == NULL || table->ndxFile == NULL) {
		return FAILURE;
	}
    else {
		int rec = 0;
		fread(&rec,sizeof(int),1,table->ndxFile);
		fread(table->ndxArray,sizeof(struct Table_ndx),rec,table->ndxFile);
		table->rec_count = rec;
		table->table_status = DB_OPEN;
		fclose(table->ndxFile);
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
		if(table->ndxArray[idx].key == key && table->ndxArray[idx].is_deleted == false){
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
		if(table->ndxArray[idx].key == key && table->ndxArray[idx].is_deleted == false){
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
			for(int idx = 0;idx<db_info.rel_count;idx++){
				delete_relation(db_info.rInfo[idx].rel_name,key,table_name);
			}
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
	table->ndxFile = fopen(table->ndx_name,"rb+");
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
	for(int idx = 0;idx<db_info.rel_count;idx++){
		int is_success = close_relation(&db_info.rInfo[idx]);
		if(is_success == FAILURE) return FAILURE;
	}
	db_info.db_status = DB_CLOSE;
	FILE *dbptr = fopen(db_info.dbname,"rb+");
	if(dbptr == NULL)return FAILURE;
	fwrite(&db_info,sizeof(struct DBInfo),1,dbptr);
	fclose(dbptr);
	return SUCCESS;
}

int create_relation(char * relation_name,char * primary_table,char * secondary_table){
	char dbfilename[200];
	strcpy(dbfilename,relation_name);
	strcat(dbfilename,".rel");
	FILE *fptr = fopen(dbfilename, "wb");
	if(fptr == NULL){
		return FAILURE;
	}
	else{
		fclose(fptr);
		struct RelationInfo * relation = &db_info.rInfo[db_info.rel_count];
		strcpy(relation->rel_name,dbfilename);
		strcpy(relation->primary_table,primary_table);
		strcpy(relation->secondary_table,secondary_table);
		relation->rel_status = DB_CLOSE;
		int status = open_relation(relation);
		if(status == SUCCESS){
			db_info.rel_count++;
			return status;
		}
		return FAILURE;
	}
}

int open_relation(struct RelationInfo *relation){
	FILE *fptr = fopen(relation->rel_name, "rb+");
	if(fptr == NULL){
		return FAILURE;
	}
	else{
		relation->relFile = fptr;
		fseek(fptr,0,SEEK_SET);
		if(get_tableInfo(relation->primary_table) == NULL || get_tableInfo(relation->secondary_table) == NULL){
			fclose(fptr);
			return FAILURE;
		}
		relation->rel_status = DB_OPEN;
		return SUCCESS;
	}
}
//helper
bool is_key_present(char * table_name,int key){
	struct TableInfo *table = get_tableInfo(table_name);
	if(table == NULL || table->table_status == DB_CLOSE){
		return false;
	}
	for(int idx = 0; idx < table->rec_count; idx++){
		if(table->ndxArray[idx].key == key && table->ndxArray[idx].is_deleted == false){
			return true;
		}
	}
	return false;
}
//helper
struct RelationInfo * get_relationInfo(char* relname){
	for(int idx = 0;idx<db_info.rel_count;idx++){
		if(strcmp(db_info.rInfo[idx].rel_name,relname)==0){
			return (&db_info.rInfo[idx]);
		}
	}
	return NULL;
}
int store_relation(char* rel_name,int primary_key,int related_key){
	char relname[100];
	strcpy(relname,rel_name);
	strcat(relname,".rel");
	struct RelationInfo *relation = get_relationInfo(relname);
	if(relation == NULL) return FAILURE;
	if(db_info.db_status == DB_CLOSE || relation->rel_status == DB_CLOSE || !is_key_present(relation->primary_table,primary_key) || !is_key_present(relation->secondary_table,related_key)){
		return FAILURE;
	}
	else{
		
		struct RelationPair pair = {primary_key,related_key,false};
		if(relation->relFile == NULL)return FAILURE;
		fseek(relation->relFile,0,SEEK_END);
		fwrite(&pair,sizeof(pair),1,relation->relFile);
		return SUCCESS;
	}
}

int get_relation(char*rel_name,void * related_rec,int search_by_key){
	char relname[100];
	strcpy(relname,rel_name);
	strcat(relname,".rel");
	struct RelationInfo *relation = get_relationInfo(relname);
	if(relation == NULL) return FAILURE;
	if(db_info.db_status == DB_CLOSE || relation->rel_status == DB_CLOSE){
		return FAILURE;
	}
	fseek(relation->relFile,0,SEEK_SET);
	struct RelationPair pair;
	int chunks_read = 0;
	while((chunks_read = fread(&pair,sizeof(pair),1,relation->relFile))>0){
		if(pair.primary_key == search_by_key && pair.is_deleted == false){
			return get_table(pair.related_key,related_rec,relation->secondary_table);
		}
	}	
	return FAILURE;
}

int delete_relation(char*rel_name,int key,char * tabel_name){
	char relname[100];
	strcpy(relname,rel_name);
	strcat(relname,".rel");
	struct RelationInfo *relation = get_relationInfo(relname);
	if(relation == NULL) return FAILURE;
	if(db_info.db_status == DB_CLOSE || relation->rel_status == DB_CLOSE){
		return FAILURE;
	}
	bool is_primary = false;
	if(strcmp(tabel_name,relation->primary_table)==0){
		is_primary = true;
	}
	if(strcmp(tabel_name,relation->secondary_table)!=0 && is_primary == false){
		return FAILURE;
	}
	fseek(relation->relFile,0,SEEK_SET);
	struct RelationPair pair;
	int chunks_read = 0;
	while((chunks_read = fread(&pair,sizeof(pair),1,relation->relFile))>0){
		if(is_primary && pair.primary_key == key && pair.is_deleted == false){
			fseek(relation->relFile,-sizeof(pair),SEEK_CUR);
			pair.is_deleted = true;
			fwrite(&pair,sizeof(pair),1,relation->relFile);
			return SUCCESS;
		}
		else if(!is_primary && pair.related_key == key && pair.is_deleted == false){
			fseek(relation->relFile,-sizeof(pair),SEEK_CUR);
			pair.is_deleted = true;
			fwrite(&pair,sizeof(pair),1,relation->relFile);
			return SUCCESS;
		}
	}
	return FAILURE;
}

int close_relation(struct RelationInfo * relation){
	if(db_info.db_status == DB_CLOSE || relation->rel_status == DB_CLOSE){
		relation->rel_status = DB_CLOSE;
		return FAILURE;
	}
	fclose(relation->relFile);
	relation->rel_status = DB_CLOSE;
	return SUCCESS;
}
