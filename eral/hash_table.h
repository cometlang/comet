#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

typedef struct eral_hash_table eral_HashTable_t;

typedef void (*eral_HashNodeDestructor_fn)(uintptr_t);

eral_HashTable_t *eral_HashTableCreate(void);

eral_HashTable_t *eral_HashTableCreateSize(uint32_t size);

void eral_HashTableDelete(eral_HashTable_t *table, eral_HashNodeDestructor_fn destructor);

void eral_HashTableInsert(eral_HashTable_t *table, const void *key,
                          uint16_t key_len, uintptr_t data);

uintptr_t eral_HashTableFind(eral_HashTable_t *table, const void *key, uint16_t key_len);

uintptr_t eral_HashTableRemove(eral_HashTable_t *table, const void *key, uint16_t key_len);

#endif /* _HASH_TABLE_H_ */
