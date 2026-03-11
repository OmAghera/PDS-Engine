#include "pds1.h"

struct DBInfo db_info;

int create_db(char *dbname){
	char dbfilename[50];
	char ndxfilename[50];
	strcpy(dbfilename,dbname);
	strcpy(ndxfilename,dbname);
	strcat(dbfilename,".dat");
	strcat(ndxfilename,".ndx");
	db_info.rec_count = 0;
	FILE *fptr = fopen(dbfilename, "wb");
	FILE *ndxfile = fopen(ndxfilename,"wb"); 
	if(fptr == NULL || ndxfile == NULL) return FAILURE;
	else {
		db_info.status = DB_CLOSE;
		fwrite(&db_info.rec_count,sizeof(int),1,ndxfile);
		fclose(fptr);
		fclose(ndxfile);
		printf("file create success\n");
		return SUCCESS;
	}
}
int open_db(char *dbname,int rec_size){
	char dbfilename[50];
	char ndxfilename[50];
	strcpy(dbfilename,dbname);
	strcpy(ndxfilename,dbname);
	strcat(dbfilename,".dat");
	strcat(ndxfilename,".ndx");
	db_info.fptr = fopen(dbfilename,"rb+");
	db_info.ndx_file = fopen(ndxfilename,"rb+");
	if(db_info.fptr == NULL || db_info.ndx_file == NULL) {
		db_info.status = DB_CLOSE;
		return FAILURE;
	}
    else {
		strcpy(db_info.dbname,dbfilename);
		strcpy(db_info.ndx_name,ndxfilename);
		db_info.rec_size = rec_size;
		struct Db_ndx temp;
		int rec = 0;
		fread(&rec,sizeof(int),1,db_info.ndx_file);
		fread(db_info.ndxArray,sizeof(struct Db_ndx),rec,db_info.ndx_file);
		db_info.rec_count = rec;
		db_info.status = DB_OPEN;
		fclose(db_info.ndx_file);
		printf("file open success\n");
        return SUCCESS;
	}
}

int store_db(int key,void* c){
	if(db_info.status == DB_CLOSE) return FAILURE;
	fseek(db_info.fptr,0,SEEK_END);
	struct Db_ndx temp = {key,ftell(db_info.fptr),false,-1};
	db_info.ndxArray[db_info.rec_count++] = temp;
	fwrite(&key,sizeof(int),1,db_info.fptr);
	fwrite(c,db_info.rec_size,1,db_info.fptr);
	printf("store success\n");
	return SUCCESS;
}

int get_db(int key,void* coutput){
	if(db_info.status == DB_CLOSE) return FAILURE;
	int location = -1;
	for(int idx = 0;idx<db_info.rec_count;idx++){
		if(db_info.ndxArray[idx].key == key){
			location = db_info.ndxArray[idx].loc;
		}
	}
	if(location == -1) return FAILURE;
	fseek(db_info.fptr,location+sizeof(int),SEEK_SET);
	fread(coutput,db_info.rec_size,1,db_info.fptr);
	return SUCCESS;
}
int update_db(int key,void *new){
	if(db_info.status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<db_info.rec_count;idx++){
		if(db_info.ndxArray[idx].key == key){
			fseek(db_info.fptr,db_info.ndxArray[idx].loc + sizeof(int),SEEK_SET);
			fwrite(new,db_info.rec_size,1,db_info.fptr);
			return SUCCESS;
		}
	}
	return FAILURE;
}
int delete_db(int key){
	if(db_info.status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<db_info.rec_count;idx++){
		if(db_info.ndxArray[idx].key == key){
			db_info.ndxArray[idx].old_key = key;
			db_info.ndxArray[idx].key = -1;
			db_info.ndxArray[idx].is_deleted = true;
			return SUCCESS;
		}
	}
	return FAILURE;
}
int undelete_db(int old_key){
	if(db_info.status == DB_CLOSE) return FAILURE;
	for(int idx = 0;idx<db_info.rec_count;idx++){
		if(db_info.ndxArray[idx].is_deleted == true && db_info.ndxArray[idx].old_key == old_key){
			db_info.ndxArray[idx].is_deleted = false;
			db_info.ndxArray[idx].key = old_key;
			db_info.ndxArray[idx].old_key = -1;
			return SUCCESS;
		}
	}
	return FAILURE;
}
int close_db(){
	if(db_info.status == DB_CLOSE) return FAILURE;
	db_info.ndx_file = fopen(db_info.ndx_name,"wb");
	//fseek(db_info.ndx_file,0,SEEK_SET);
	if(db_info.ndx_file == NULL){
		return FAILURE;
	}
	fwrite(&db_info.rec_count,sizeof(int),1,db_info.ndx_file);
	fwrite(db_info.ndxArray,sizeof(struct Db_ndx),db_info.rec_count,db_info.ndx_file);
	fclose(db_info.fptr);
	fclose(db_info.ndx_file);
	printf("files closed success\n");
	db_info.status = DB_CLOSE;
	return SUCCESS;
}
