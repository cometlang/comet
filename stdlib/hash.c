#include "comet.h"
#include "cometlib.h"

#include <stdlib.h>

#define MAX_LOAD_PERCENTAGE 0.75

typedef struct hash_entry
{
    VALUE key;
    VALUE value;
    struct hash_entry *next;
} HashEntry;

typedef struct
{
    int count;
    int capacity;
    HashEntry *entries;
} HashTable;

void *hash_constructor(void)
{
    HashTable *data = ALLOCATE(HashTable, 1);
    data->count = 0;
    data->capacity = 0;
    data->entries = NULL;
    return data;
}

void hash_destructor(void *data)
{
    FREE(HashTable, data);
}

VALUE hash_add(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashTable *data = (HashTable *) AS_NATIVE_INSTANCE(self)->data;
    if (data->count >= data->capacity * MAX_LOAD_PERCENTAGE)
    {
        int old_capacity = data->capacity;
        data->capacity = GROW_CAPACITY(data->capacity);
        data->entries = GROW_ARRAY(data->entries, HashEntry, old_capacity, data->capacity);
        for (int i = old_capacity; i < data->capacity; i++)
        {
            data->entries[i].key = NIL_VAL;
            data->entries[i].value = NIL_VAL;
            data->entries[i].next = NULL;
        }
    }
    uint32_t hash = 0;
    // Get the hash value of the key...
    int index = hash % data->capacity;
    if (data->entries[index].key == NIL_VAL)
    {
        // data->entries[index].key = key
    }
    else
    {
        HashEntry *current = &data->entries[index];
        while (current->next != NULL)
        {
            current = current->next;
        }
    }
    data->count++;
    return NIL_VAL;
}

VALUE hash_remove(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_iterable_contains_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_iterable_empty_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_iterable_iterator(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_obj_to_string(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_get(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_set(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_hash(void)
{
    VALUE klass = defineNativeClass("Hash", hash_constructor, hash_destructor, NULL);
    defineNativeMethod(klass, &hash_add, "add", false);
    defineNativeMethod(klass, &hash_remove, "remove", false);
    defineNativeMethod(klass, &hash_iterable_contains_q, "contains?", false);
    defineNativeMethod(klass, &hash_iterable_empty_q, "empty?", false);
    defineNativeMethod(klass, &hash_iterable_iterator, "iterator", false);
    defineNativeMethod(klass, &hash_obj_to_string, "to_string", false);
    defineNativeOperator(klass, &hash_get, OPERATOR_INDEX);
    defineNativeOperator(klass, &hash_set, OPERATOR_INDEX_ASSIGN);
}