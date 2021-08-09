#include "hashtable.h"
#include "list.h"

#define hash_table_hash(ht,k)   (ht->hash_func(ht,k))
#define hash_table_comp(ht,e,k) (ht->comp_func(ht,e,k))

HashTable*  hash_table_new(
    int node_size, 
    AllocFunc alloc_func, 
    CompFunc comp_func, 
    HashFunc hash_func, 
    ValueFunc value_func,
    int array_size){

    HashTable* hash_table = malloc(sizeof(HashTable) + node_size * array_size);

    hash_table->alloc_func = alloc_func;
    hash_table->comp_func = comp_func;
    hash_table->hash_func = hash_func;
    hash_table->node_size = node_size;
    hash_table->array_size = array_size;
    hash_table->value_func = value_func;

    memset(hash_table->entries,0, node_size * array_size);

    return hash_table;

}

void* hash_table_put(HashTable* hash_table, void* key, void* value){

    int hash = hash_table_hash(hash_table, key) % hash_table->array_size;
}

HTEntry* hash_table_get (HashTable* hash_table, void* key){

    int hash = hash_table_hash(hash_table, key) % hash_table->array_size;

    for (HTEntry* entry = &(hash_table->entries[hash]); entry; entry = entry->next){

        if (!hash_table_comp(hash_table, entry, key)){
            return entry;
        }
    }
    return NULL;
}

void hash_table_delete (HashTable* hash_table, void* key){
}

void hash_table_free (HashTable* hash_table){

    for (int i=0;i<hash_table->array_size;i++){
        for (HTEntry* e = hash_table[i]->next; e; e = e->next){
            free(e);
        }

    }
    free(hash_table);
}

HashTable* str_hash_table_new (int key_size, int value_size, int array_size){

    HashTable* hash_table = hash_table_new(
        sizeof(HTEntry) + key_size + value_size,
        str_alloc,
        str_comp,
        str_hash,
        array_size
    );

    hash_table->reserved[0] = key_size;
    hash_table->reserved[1] = value_size;

    return hash_table;
}
