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
#define MAX 100000
struct Table_ndx{
	int key;
	int loc;
	bool is_deleted;
	int old_key;
};
struct TableInfo{
    char tname[100];
	char ndx_name[100];
    int rec_size;
    FILE * tFile;
    FILE *ndxFile;
    struct Table_ndx ndxArray[MAX];
    int table_status;
    int rec_count;
};
struct RelationInfo{
    char rel_name[100];
    char primary_table[100];
    char secondary_table[100];
    FILE *relFile;
};
struct RelationPair{
    int primary_key;
    int related_key;
    bool is_deleted;
};
struct DBInfo{
    struct TableInfo tinfo[2];
    struct RelationInfo rInfo;
    int num_table;   
    int db_status;
    int rel_status;
};
extern struct DBInfo db_info;
struct TableInfo* get_tableInfo(char* name); //helper function for finding tableInfo using table name
int create_db(char* table1_name,char* table2_name); //create an empty file by doing fopen with wb then fclose
int create_table(char *table_name);
int open_db(char *table1_name,char* table2_name,int rec1_size,int rec2_size); //fopen in rb+
int open_table(char* table_name,int rec_size);
int store_table(int key,void *c,char* table_name); //fseek to the end of the file then fwrite
int get_table(int key,void* coutput,char* table_name); //fseek to the begining iterate over each record untile matching rec is found or EOF return 0 if found else return 1
int update_table(int key,void *new,char* table_name);
int close_db(); //fclose
int close_table(struct TableInfo* table);
int delete_table(int key,char * table_name);
int undelete_table(int old_key,char* table_name);


//functions for relation management
int create_relation(char* relation_name,char * primary_table,char * secondary_table);
int open_relation(char * relation_name);
bool is_key_present(char * table_name,int key);
int store_relation(int primary_key,int related_key);
//asuming one-one relation
int get_relation(void * related_rec,int search_rel_key);
int delete_relation(int key,char * table_name);
int close_relation();
