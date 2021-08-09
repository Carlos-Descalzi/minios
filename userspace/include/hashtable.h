#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

typedef struct HTEntry{
    struct HTEntry* next;
} HTEntry;

typedef HTEntry* (*AllocFunc)    (HashTable*, void*, void*);
typedef int      (*CompFunc)     (HashTable*, HTEntry*, void*);
typedef int      (*HashFunc)     (HashTable*, void*);

typedef struct {
    AllocFunc alloc_func;
    CompFunc comp_func;
    HashFunc hash_func;
    int node_size;
    int array_size;
    int reserved[4];
    HTEntry entries[];
} HashTable;

HashTable*  hash_table_new      (int node_size, AllocFunc alloc_func, CompFunc comp_func, 
                                HashFunc hash_func, int array_size);
HashTable*  str_hash_table_new  (int key_size, int value_size, int array_size);
void*       hash_table_put      (HashTable* hash_table, void* key, void* value);
void*       hash_table_get      (HashTable* hash_table, void* key);
void        hash_table_delete   (HashTable* hash_table, void* key);
void        hash_table_free     (HashTable* hash_table);


#endif
