#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "liberal.h"
#include "hash_table.h"

#define DEFAULT_HASH_TABLE_LENGTH 4096

struct hash_table_node {
    uintptr_t data;
    void *key;
    uint32_t key_len;
    struct hash_table_node *next;
};

struct eral_hash_table
{
    struct hash_table_node **table;
    uint32_t size;
    uint32_t numItems;
};

static uint32_t elf_hash(const void *key, uint16_t key_len, uint32_t size)
{
    const unsigned char *array = key;
    uint32_t hash = 0, mixer;
    uint16_t i;

    for (i = 0; i < key_len; i++)
    {
        hash = (hash << 4) + array[i];
        mixer = hash & 0xf0000000L;

        if (mixer != 0)
            hash ^= mixer >> 24;

        hash &= ~mixer;
    }

    return hash % size;
}

eral_HashTable_t *eral_HashTableCreate(void)
{
    return eral_HashTableCreateSize(DEFAULT_HASH_TABLE_LENGTH);
}

eral_HashTable_t *eral_HashTableCreateSize(uint32_t size)
{
    eral_HashTable_t *result = (eral_HashTable_t *) malloc(sizeof(eral_HashTable_t));
    result->table = (struct hash_table_node **) malloc(sizeof(void *) * size);
    result->size = size;

    memset(result->table, 0, sizeof(void *) * size);
    return result;
}

void destroy_node(struct hash_table_node *node)
{
    if (node->key != NULL)
    {
        free(node->key);
    }
    free(node);
}

void eral_HashTableDelete(eral_HashTable_t *table, eral_HashNodeDestructor_fn destructor)
{
    uint32_t i;
    for (i = 0; i < table->size; i++)
    {
        if (table->table[i] != NULL)
        {
            struct hash_table_node *current = table->table[i];
            struct hash_table_node *next;
            while (current != NULL)
            {
                next = current->next;
                if (destructor)
                {
                    destructor(current->data);
                }
                destroy_node(current);
                current = next;
            }
            table->table[i] = NULL;
        }
    }
    free(table->table);
    free(table);
}

void eral_HashTableInsert(eral_HashTable_t *table, const void *key, uint16_t key_len, uintptr_t data)
{
    uint32_t index = elf_hash(key, key_len, table->size);
    struct hash_table_node *node = (struct hash_table_node *) malloc(sizeof(struct hash_table_node));
    node->key = malloc(key_len);
    node->data = data;
    memcpy(node->key, key, key_len);
    node->key_len = key_len;
    node->next = NULL;
    if (table->table[index] == NULL)
    {
        table->table[index] = node;
    }
    else
    {
        struct hash_table_node *temp = table->table[index];
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = node;
    }
    table->numItems++;
}

uintptr_t eral_HashTableFind(eral_HashTable_t *table, const void *key, uint16_t key_len)
{
    uint32_t index = elf_hash(key, key_len, table->size);
    struct hash_table_node *result = table->table[index];
    while (result != NULL)
    {
        if (key_len == result->key_len &&
            memcmp(key, result->key, key_len) == 0)
        {
            return result->data;
        }
        else
        {
            result = result->next;
        }
    }
    return NULL_DATA;
}

uintptr_t eral_HashTableRemove(eral_HashTable_t *table, const void *key, uint16_t key_len)
{
    uint32_t index = elf_hash(key, key_len, table->size);
    struct hash_table_node *current = table->table[index];
    struct hash_table_node *previous = NULL;
    uintptr_t result = NULL_DATA;
    while (current != NULL)
    {
        if (current->key_len == key_len &&
            memcmp(current->key, key, key_len) == 0)
        {
            result = current->data;
            if (previous == NULL)
            {
                table->table[index] = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            destroy_node(current);
            table->numItems--;
            return result;
        }   
        previous = current;
        current = current->next;
    }
    return result;
}
