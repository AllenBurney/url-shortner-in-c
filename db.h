#ifndef DB_H
#define DB_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>

#define MAX_ENTRIES 100


typedef struct{
    char short_url[10];
    char long_url[254];
}URLMap;
extern int db_entries;
extern URLMap db[MAX_ENTRIES];

//void init_storage();
void save_info(const char* new_url, const char* old_url);

char* find_long_url(const char* new_url);

#endif