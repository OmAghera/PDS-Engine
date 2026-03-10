#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#pragma once
#define SUCCESS 0
#define FAILURE 1
#define REC_NOT_FOUND 1
#define DB_OPEN 0
#define DB_CLOSE 1
struct Db_ndx{
	int key;
	int loc;
	bool is_deleted;
	int old_key;
};
struct DBInfo{
	FILE * fptr;
	int rec_size;
	char dbname[50];
	int status;
	struct Db_ndx ndxArray[100000];
	int rec_count;
	FILE *ndx_file;
	char ndx_name[50];
};
void coursedb_init(); 
extern struct DBInfo cdb_info;
int create_db(char* db_name); //create an empty file by doing fopen with wb then fclose
int open_db(char *dbname,int rec_size); //fopen in rb+
int store_db(int key,void *c); //fseek to the end of the file then fwrite
int get_db(int key,void* coutput); //fseek to the begining iterate over each record untile matching rec is found or EOF return 0 if found else return 1
int update_db(int key,void *new);
int close_db(); //fclose
int delete_db(int key);
int undelete_db(int key);
