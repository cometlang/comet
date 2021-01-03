#include "comet.h"
#include "cometlib.h"

#include <stdlib.h>

typedef struct
{
    VALUE key;
    VALUE value;
} HashEntry;

typedef struct
{
    int count;
    int capacity;
    HashEntry *entries;
} HashTable;

void *hash_constructor(void)
{
    HashTable *data = (HashTable *)malloc(sizeof(HashTable));
    data->count = 0;
    data->capacity = -1;
    data->entries = NULL;
    return data;
}

void hash_destructor(void *data)
{
    free(data);
}

VALUE hash_add(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
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
    defineNativeMethod(klass, &hash_obj_to_string, "to_string", false);
    defineNativeOperator(klass, &hash_get, OPERATOR_INDEX);
    defineNativeOperator(klass, &hash_set, OPERATOR_INDEX_ASSIGN);
}