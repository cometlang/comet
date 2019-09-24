#include <stdlib.h>
#include <string.h>

#define ERAL_DEBUG 1
#include "eral_config.h"
#include "liberal.h"
#include "hash_table.h"

static void test_Insert(void)
{
    printf("Running test_Insert...");
    eral_HashTable_t *table = eral_HashTableCreate();

    eral_HashTableInsert(table, "Hello", 5, (uintptr_t)42);

    eral_HashTableDelete(table, NULL);
    printf("ok\n");
}

static void test_Find(void)
{
    printf("Running test_Find...");
    const char *key = "Hello";
    size_t key_len = strlen(key);
    eral_HashTable_t *table = eral_HashTableCreate();

    eral_HashTableInsert(table, key, key_len, (uintptr_t)42);
    int result = (int)eral_HashTableFind(table, key, key_len);
    ASSERT(result == 42);
    result = (int)eral_HashTableFind(table, key, key_len);
    ASSERT(result == 42);

    eral_HashTableDelete(table, NULL);
    printf("ok\n");
}

static void test_Remove(void)
{
    printf("Running test_Remove...");
    const char *key = "Hello";
    size_t key_len = strlen(key);
    eral_HashTable_t *table = eral_HashTableCreate();

    eral_HashTableInsert(table, key, key_len, (uintptr_t)42);
    int result = (int)eral_HashTableRemove(table, key, key_len);
    ASSERT(result == 42);
    result = (int)eral_HashTableRemove(table, key, key_len);
    ASSERT(result == NULL_DATA);

    eral_HashTableDelete(table, NULL);
    printf("ok\n");
}

typedef struct test_struct_thing
{
    const char *key;
    uint32_t key_len;
} test_StructThing_t;

void deleteStructThing(uintptr_t data)
{
    free((test_StructThing_t *)data);
}

void test_DeleteWithDynamicMemory(void)
{
    printf("Running test_Delete With Dynamic Memory...");
    const char *key = "Hello";
    size_t key_len = strlen(key);
    eral_HashTable_t *table = eral_HashTableCreate();
    test_StructThing_t *data = (test_StructThing_t *)malloc(sizeof(test_StructThing_t));
    data->key = key;
    data->key_len = key_len;

    eral_HashTableInsert(table, key, key_len, (uintptr_t)data);
    test_StructThing_t *result = (test_StructThing_t *)eral_HashTableFind(table, key, key_len);
    ASSERT(result->key_len == key_len);
    ASSERT(strncmp(result->key, key, key_len) == 0);

    eral_HashTableDelete(table, deleteStructThing);
    printf("ok\n");
}

int main(void)
{
    test_Insert();
    test_Find();
    test_Remove();
    test_DeleteWithDynamicMemory();
    return EXIT_SUCCESS;
}
