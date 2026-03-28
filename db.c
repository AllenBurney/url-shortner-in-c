#include "db.h"


int db_entries = 0;
URLMap db[MAX_ENTRIES];


void save_info(const char* new_url, const char* old_url){
    if(db_entries >= MAX_ENTRIES) return ;
    strcpy(db[db_entries].short_url, new_url);
    strcpy(db[db_entries].long_url, old_url);
    db_entries++;
    return ;
}

char* find_long_url(const char* new_url){
    for(int i = 0; i<db_entries; i++){
        if(strcmp(db[i].short_url, new_url) == 0){
            return db[i].long_url;
        }
    }
    return NULL;
}


