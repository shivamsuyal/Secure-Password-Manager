#ifndef dictionary_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


typedef struct dict_ent{
    char *org;
    char *usr;
    char *psk;
    struct dict_ent *next;
}dict_ent;

typedef struct dict_t{
    dict_ent **entries;
}dict_t;

typedef struct multi2entry{
    struct multi2entry * prev;
    char *e1;
    char *e2;
}multi2entry;


int genHash(const char *s1,const int size){
    union { uint64_t h; uint8_t u[8]; } uu;
    int i=0; uu.h=strlen(s1);
    while (*s1){
        uu.u[i%8] += *s1 + i + (*s1 >> ((uu.h/(i+1)) % 5));
        s1++;
        i++;
    }
    return uu.h % size; //64-bit
    //return (uu.h+(uu.h>>32)) % size; //32-bit
}

dict_t * intDict(const int size){
    dict_t * dTable = (dict_t *)malloc(sizeof(dict_t));
    dTable->entries = (dict_ent **)malloc(sizeof(dict_ent)*size);
    for(int i=0; i < size; i++){
        dTable->entries[i]=NULL;
    }
    return dTable;
}


void dictAdd(dict_t *dTable,const char *org,const char *usr,const char *psk,const int size){
    int hashId = genHash(org,size);
    dict_ent *entry = dTable->entries[hashId];
    
    if(entry == NULL){
        entry = (dict_ent *)malloc(sizeof(dict_ent));
        entry->org = (char *)malloc(sizeof(org)+1);
        entry->usr = (char *)malloc(sizeof(usr)+1);
        entry->psk = (char *)malloc(sizeof(psk)+1);
        entry->next=NULL;
        strcpy(entry->org,org);
        strcpy(entry->usr,usr);
        strcpy(entry->psk,psk);
        dTable->entries[hashId]=entry;
        return;
    }
    
    dict_ent *prev;

    while(entry != NULL){
        if(strcmp(org,entry->org) == 0 && strcmp(usr,entry->usr) == 0){
            free(entry->psk);
            entry->psk = (char *)malloc(strlen(psk)+1);
            strcpy(entry->psk,psk);
            return;
        }
        prev = entry;
        entry = entry->next;
    }

    entry = (dict_ent *)malloc(sizeof(dict_ent));
    entry->org = (char *)malloc(sizeof(org)+1);
    entry->usr = (char *)malloc(sizeof(usr)+1);
    entry->psk = (char *)malloc(sizeof(psk)+1);
    entry->next=NULL;
    strcpy(entry->org,org);
    strcpy(entry->usr,usr);
    strcpy(entry->psk,psk);
    prev->next = entry;
}
    
void dictDelete(dict_t *dTable,const char *org,const char *usr,const int size){
    int hashId = genHash(org,size);
    dict_ent * entry = dTable->entries[hashId];
    dict_ent *prev = NULL;

    while(entry != NULL){
        if(strcmp(org,entry->org) == 0 && strcmp(usr,entry->usr) == 0){
            if(prev == NULL){ // True [1] if only one element is preset |or| [2] if it's a 1st element
                if(entry->next == NULL){ // if only one element is present
                    free(dTable->entries[hashId]);
                    dTable->entries[hashId] = NULL;
                    break;
                }
                // if it's a 1st element
                dTable->entries[hashId] = entry->next;
                free(entry);
            }else{
                if(entry->next != NULL){ // if the element is in between the dict
                    prev->next = entry->next;
                    free(entry);
                    break;
                }
                // if it's a last element
                free(entry);
                prev->next = NULL;
            }
        }
        prev = entry;
        entry = entry->next;
    }
}

void getPsk(dict_t *dTable,const char *org,const char *usr,const int size,char *psk){
    int hashId = genHash(org,size);
    dict_ent * entry = dTable->entries[hashId];

    while(entry != NULL){
        if(strcmp(org,entry->org) == 0 && strcmp(usr,entry->usr) == 0){
            strcpy(psk,entry->psk);
        }
        entry = entry->next;
    }
}

multi2entry * getByOrg(dict_t *dTable,const char * org,const int size){
    int hashId = genHash(org,size);
    dict_ent * entry = dTable->entries[hashId];
    multi2entry * m2e;
    multi2entry *prev = NULL;

    int i = 0;
    while(entry != NULL){
        if(strcmp(org,entry->org) == 0 ){
            m2e = (multi2entry *)malloc(sizeof(multi2entry));
            m2e->e1 = (char *)malloc(sizeof(strlen(entry->usr)+1));
            m2e->e2 = (char *)malloc(sizeof(strlen(entry->psk)+1));
            strcpy(m2e->e1,entry->usr);
            strcpy(m2e->e2,entry->psk);
            m2e->prev = prev;
            prev = m2e;
            i++;
        }
        if(i > 10){
            break;
        }
        entry = entry->next;
    }
    return m2e;
}

int chkUser(dict_t *dict,char *org,char *usr,const int size){
    int hash = genHash(org,size);
    dict_ent *entry = dict->entries[hash];
    while (entry != NULL){
        if(strcmp(usr,entry->usr) == 0){
            return 1;
        }
        entry = entry->next;
    }
    return 0;
}

void dump_all(dict_t * dict,const int size){
    dict_ent *entry;
    for(int i=0; i < size; i++){
        entry = dict->entries[i];
        if(entry == NULL){
            continue;
        }
        while(entry != NULL){
            printf("%s:\n\t%s ::: %s\n",entry->org,entry->usr,entry->psk);
            entry=entry->next;
        }
        printf("\n\n");
    }
}



#define dictionary_h
#endif