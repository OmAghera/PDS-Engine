#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pds3.h"

typedef struct Hospital {
    int hospital_id;
    char name[100];
    char address[200];
    char email[50];
} Hospital;

#define TREPORT(a1,a2) do { printf("Status: %s - %s\n\n", a1, a2); fflush(stdout); } while(0)
void process_line( char *test_case );

// --- PDS Core Function Wrappers ---
int pds_create(char* tname1, char* tname2) { return create_db(tname1, tname2); }
int pds_open(char* tname1, int rec_size1, char* tname2, int rec_size2) { return open_db(tname1, tname2, rec_size1, rec_size2); }
int add_record(char* tname, int key, void* record) { return store_table(key, record, tname); }
int search_record(char* tname, int key, void* record) { return get_table(key, record, tname); }
int update_record(char* tname, int key, void* record) { return update_table(key, record, tname); }
int delete_record(char* tname, int key) { return delete_table(key, tname); }
int undelete_record(char* tname, int key) { return undelete_table(key, tname); }
int pds_close() { return close_db(); }

// --- Main ---
int main(int argc, char *argv[]) {
    FILE *cfptr;
    char test_case[500];

    if( argc != 2 ){
        fprintf(stderr, "Usage: %s testcasefile\n", argv[0]);
        exit(1);
    }

    cfptr = (FILE *) fopen(argv[1], "r");
    if(!cfptr){
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(1);
    }

    while(fgets(test_case, sizeof(test_case)-1, cfptr)){
        if( !strcmp(test_case,"\n") || !strcmp(test_case,"") ) continue;
        test_case[strcspn(test_case, "\n")] = 0; 
        printf("Test case: %s\n", test_case); fflush(stdout);
        process_line( test_case );
    }
    
    fclose(cfptr);
    return 0;
}

void process_line( char *test_case ) {
    // FIXED: Increased buffer sizes to 100 to prevent stack buffer over-reads by pds3.c's fwrite
    char tname1[100], tname2[100], rel_name[100];
    char command[20], param1[15], param2[15], info[1024];
    int id1, id2, status, rec_size, expected_status;
    Hospital testHospital;

    rec_size = sizeof(Hospital);
    sscanf(test_case, "%s", command);

    // --- CORE DB COMMANDS ---
    if( !strcmp(command,"CREATE") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, tname2, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        status = pds_create(tname1, tname2);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"pds_create returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"OPEN") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, tname2, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        status = pds_open(tname1, rec_size, tname2, rec_size);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"pds_open returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command, "STORE") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, param1, param2);
        if( !strcmp(param2,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        sscanf(param1, "%d", &id1);
        testHospital.hospital_id = id1;
        char name[100], address[200], email[50];
        sscanf(test_case, "%*s %*s %*s %*s %s %s %s", name, address, email);
        strcpy(testHospital.name, name);
        strcpy(testHospital.address, address);
        strcpy(testHospital.email, email);
        status = add_record(tname1, id1, &testHospital);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"add_record returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"SEARCH") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, param1, param2);
        char expected_name[100], expected_address[200], expected_email[50];
        if( !strcmp(param2,"0") ){
            sscanf(test_case, "%*s %*s %*s %*s %s %s %s", expected_name, expected_address, expected_email);
            expected_status = SUCCESS;
        } else expected_status = REC_NOT_FOUND;
        sscanf(param1, "%d", &id1);
        status = search_record(tname1, id1, &testHospital);
        if( status != expected_status ){
            sprintf(info,"search key: %d; Got status %d",id1, status); TREPORT("FAIL", info);
        } else {
            if( expected_status == SUCCESS ){
                if (testHospital.hospital_id == id1 && strcmp(testHospital.name, expected_name) == 0 &&
                    strcmp(testHospital.address, expected_address) == 0 && strcmp(testHospital.email, expected_email) == 0) {
                        TREPORT("PASS", "");
                } else {
                    sprintf(info,"Data mismatch... Expected:{%d,%s,%s,%s} Got:{%d,%s,%s,%s}\n", id1, expected_name, expected_address, expected_email, testHospital.hospital_id, testHospital.name, testHospital.address, testHospital.email);
                    TREPORT("FAIL", info);
                }
            } else TREPORT("PASS", "");
        }
    }
    else if( !strcmp(command,"UPDATE") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, param1, param2);
        if( !strcmp(param2,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        sscanf(param1, "%d", &id1);
        testHospital.hospital_id = id1;
        char name[100], address[200], email[50];
        sscanf(test_case, "%*s %*s %*s %*s %s %s %s", name, address, email);
        strcpy(testHospital.name, name);
        strcpy(testHospital.address, address);
        strcpy(testHospital.email, email);
        status = update_record(tname1, id1, &testHospital);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"update_record returned status %d",status); TREPORT("FAIL", info); }
    }
    else if(!strcmp(command,"DELETE") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, param1, param2);
        if( !strcmp(param2,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        sscanf(param1, "%d", &id1);
        status = delete_record(tname1, id1);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"delete_record returned status %d",status); TREPORT("FAIL", info); }
    }
    else if(!strcmp(command,"UNDELETE") ){
        sscanf(test_case, "%s %s %s %s", command, tname1, param1, param2);
        if( !strcmp(param2,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        sscanf(param1, "%d", &id1);
        status = undelete_record(tname1, id1);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"undelete_record returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"CLOSE") ){
        sscanf(test_case, "%s %s", command, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        status = pds_close();
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"pds_close returned status %d",status); TREPORT("FAIL", info); }
    }

    // --- NEW RELATIONAL COMMANDS ---
    else if( !strcmp(command,"CREATE_REL") ){
        sscanf(test_case, "%s %s %s %s %s", command, rel_name, tname1, tname2, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        status = create_relation(rel_name, tname1, tname2);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"create_relation returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"OPEN_REL") ){
        sscanf(test_case, "%s %s %s", command, rel_name, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        status = open_relation(rel_name);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"open_relation returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"STORE_REL") ){
        char pk_str[15], rk_str[15];
        sscanf(test_case, "%s %s %s %s", command, pk_str, rk_str, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        sscanf(pk_str, "%d", &id1);
        sscanf(rk_str, "%d", &id2);
        status = store_relation(id1, id2);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"store_relation returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"SEARCH_REL") ){
        sscanf(test_case, "%s %s %s", command, param1, param2);
        char expected_name[100], expected_address[200], expected_email[50];
        if( !strcmp(param2,"0") ){
            sscanf(test_case, "%*s %*s %*s %s %s %s", expected_name, expected_address, expected_email);
            expected_status = SUCCESS;
        } else expected_status = FAILURE;
        sscanf(param1, "%d", &id1);
        status = get_relation(&testHospital, id1);
        if( status != expected_status ){
            sprintf(info,"search_relation key: %d; Got status %d", id1, status); TREPORT("FAIL", info);
        } else {
            if( expected_status == SUCCESS ){
                if (strcmp(testHospital.name, expected_name) == 0 && strcmp(testHospital.address, expected_address) == 0 && strcmp(testHospital.email, expected_email) == 0) {
                        TREPORT("PASS", "");
                } else {
                    sprintf(info,"Data mismatch in related record... Expected:{%s,%s,%s} Got:{%s,%s,%s}\n", expected_name, expected_address, expected_email, testHospital.name, testHospital.address, testHospital.email);
                    TREPORT("FAIL", info);
                }
            } else TREPORT("PASS", "");
        }
    }
    else if( !strcmp(command,"DELETE_REL") ){
        sscanf(test_case, "%s %s %s %s", command, param1, tname1, param2);
        if( !strcmp(param2,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        sscanf(param1, "%d", &id1);
        status = delete_relation(id1, tname1);
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"delete_relation returned status %d",status); TREPORT("FAIL", info); }
    }
    else if( !strcmp(command,"CLOSE_REL") ){
        sscanf(test_case, "%s %s", command, param1);
        if( !strcmp(param1,"0") ) expected_status = SUCCESS; else expected_status = FAILURE;
        status = close_relation();
        if( status == expected_status ) TREPORT("PASS", "");
        else { sprintf(info,"close_relation returned status %d",status); TREPORT("FAIL", info); }
    }
    else { printf("Unknown command: %s\n\n", command); }
}