#include <stdlib.h>
#include <stdio.h>

#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"


#define TABLE_MAX_LOAD 0.75

static VALUE hash_class;
static VALUE hash_iterator_class;

typedef struct hash_entry
{
    VALUE key;
    VALUE value;
} HashEntry;

typedef struct
{
    ObjInstance obj;
    int count;
    int32_t capacity;
    HashEntry *entries;
} HashTable;

typedef struct {
    ObjInstance obj;
    HashTable *table;
    int index;
    int values_returned;
} HashIterator;

void hash_iterator_constructor(void *instanceData)
{
    HashIterator *iter = (HashIterator *)instanceData;
    iter->table = NULL;
    iter->index = 0;
    iter->values_returned = 0;
}

VALUE hash_iterator_has_next_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashIterator *iter = GET_NATIVE_INSTANCE_DATA(HashIterator, self);
    if (iter->values_returned < iter->table->count)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE hash_iterator_get_next(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashIterator *iter = GET_NATIVE_INSTANCE_DATA(HashIterator, self);
    while (IS_NIL(iter->table->entries[iter->index].key))
        iter->index++;

    VALUE result = iter->table->entries[iter->index].key;
    iter->index++;
    iter->values_returned++;
    return result;
}

VALUE hash_iterator_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashIterator *iter = GET_NATIVE_INSTANCE_DATA(HashIterator, self);
    std::stringstream stream;
    stream << "[Hash Iterator] Index: " << iter->index
           << ", values_returned: " << iter->values_returned
           << ", count: " << iter->table->count
           << ", capacity: " << iter->table->capacity;
    std::string result = stream.str();
    return copyString(vm, result.c_str(), result.length());
}

VALUE hash_iterable_contains_q(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    HashTable* data = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    VALUE contains = arguments[0];
    for (int i = 0; i < data->capacity; i++)
    {
        HashEntry entry = data->entries[i];
        if (entry.key == NIL_VAL)
            continue;
        if (compare_objects(vm, contains, data->entries[i].value))
        {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
}

VALUE hash_iterable_empty_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashTable *data = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (data->count == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE hash_iterable_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(hash_iterator_class)));
    HashIterator *iter = GET_NATIVE_INSTANCE_DATA(HashIterator, instance);
    iter->table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    return instance;
}

VALUE hash_iterable_count(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    return create_number(vm, table->count);
}

void hash_constructor(void *instanceData)
{
    HashTable *table = (HashTable *)instanceData;
    table->count = 0;
    table->capacity = -1;
    table->entries = NULL;
}

void hash_destructor(void *data)
{
    HashTable *table = (HashTable *)data;
    FREE_ARRAY(HashEntry, table->entries, table->capacity + 1);
}

VALUE hash_create(VM *vm)
{
    return OBJ_VAL(newInstance(vm, AS_CLASS(hash_class)));
}

static HashEntry *find_entry(VM *vm, HashEntry *entries, int capacity, Value key)
{
    call_function(vm, key, common_strings[STRING_HASH], 0, NULL);
    uint32_t index = ((uint32_t) number_get_value(peek(vm, 0))) & capacity;
    pop(vm);
    HashEntry *tombstone = NULL;

    for (;;)
    {
        HashEntry *entry = &entries[index];

        if (entry == NULL || entry->key == NIL_VAL)
        {
            if (IS_NIL(entry->value))
            {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                // We found a tombstone.
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        else if (compare_objects(vm, key, entry->key))
        {
            // We found the key.
            return entry;
        }

        index = (index + 1) & capacity;
    }
}

VALUE hash_find(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count == 0)
        return FALSE_VAL;

    VALUE key = arguments[0];
    HashEntry *entry = find_entry(vm, table->entries, table->capacity, key);
    if (entry == NULL || entry->key == NIL_VAL)
    {
        call_function(vm, key, common_strings[STRING_TO_STRING], 0, NULL);
        throw_exception_native(vm, "KeyNotFoundException",
            "Could not find the key '%s'", string_get_cstr(peek(vm, 0)));
        pop(vm);
        return NIL_VAL;
    }

    return entry->value;
}

VALUE hash_has_key_q(VM* vm, VALUE self, int arg_count, VALUE* arguments)
{
    HashTable* table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count == 0)
        return FALSE_VAL;

    VALUE key = arguments[0];
    HashEntry* entry = find_entry(vm, table->entries, table->capacity, key);
    if (entry == NULL || entry->key == NIL_VAL)
    {
        return FALSE_VAL;
    }
    return TRUE_VAL;
}

VALUE hash_get(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count == 0)
        return NIL_VAL;

    VALUE key = arguments[0];
    HashEntry *entry = find_entry(vm, table->entries, table->capacity, key);
    if (entry == NULL || entry->key == NIL_VAL)
    {
        if (arg_count == 2)
        {
            return arguments[1];
        }
        return NIL_VAL;
    }

    return entry->value;
}

static void adjust_capacity(VM *vm, HashTable *table, int capacity)
{
    HashEntry *entries = ALLOCATE(HashEntry, capacity + 1);
    for (int i = 0; i <= capacity; i++)
    {
        entries[i].key = NIL_VAL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    if (table->entries != NULL)
    {
        for (int32_t i = 0; i <= table->capacity; i++)
        {
            HashEntry *entry = &table->entries[i];
            if (entry == NULL || entry->key == NIL_VAL)
                continue;

            HashEntry *dest = find_entry(vm, entries, capacity, entry->key);
            dest->key = entry->key;
            dest->value = entry->value;
            table->count++;
        }
        FREE_ARRAY(HashEntry, table->entries, table->capacity + 1);
    }

    table->entries = entries;
    table->capacity = capacity;
}

VALUE hash_add(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (arguments[0] == NIL_VAL)
    {
        throw_exception_native(vm, "ArgumentException", "Hash key can't be nil");
        return NIL_VAL;
    }

    if (table->count + 1 > (table->capacity + 1) * TABLE_MAX_LOAD)
    {
        // Figure out the new table size.
        int capacity = GROW_CAPACITY(table->capacity + 1) - 1;
        adjust_capacity(vm, table, capacity);
    }

    VALUE key = arguments[0];
    VALUE value = arguments[1];
    HashEntry *entry = find_entry(vm, table->entries, table->capacity, key);

    bool isNewKey = entry->key == NIL_VAL;
    if (isNewKey && IS_NIL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

VALUE hash_remove(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count == 0)
        return FALSE_VAL;

    // Find the entry.
    VALUE key = arguments[0];
    HashEntry *entry = find_entry(vm, table->entries, table->capacity, key);
    if (entry == NULL || entry->key == NIL_VAL)
        return false;

    // Place a tombstone in the entry.
    entry->key = NIL_VAL;
    entry->value = TRUE_VAL;

    return TRUE_VAL;
}

void hash_add_all(VM *vm, HashTable *from, HashTable *to)
{
    for (int32_t i = 0; i <= from->capacity; i++)
    {
        HashEntry *entry = &from->entries[i];
        VALUE args[2] = {entry->key, entry->value};
        if (entry->key != NIL_VAL)
        {
            hash_add(vm, OBJ_VAL(to), 2, args);
        }
    }
}

void table_remove_white(VM *vm, HashTable *table)
{
    for (int32_t i = 0; i <= table->capacity; i++)
    {
        HashEntry *entry = &table->entries[i];
        if (entry->key != NIL_VAL && !AS_OBJ(entry->key)->isMarked)
        {
            hash_remove(vm, OBJ_VAL(table), 1, &entry->key);
        }
    }
}

void hash_mark_contents(VALUE self)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    for (int32_t i = 0; i <= table->capacity; i++)
    {
        HashEntry *entry = &table->entries[i];
        if (entry->key != NIL_VAL)
        {
            markValue(entry->key);
            markValue(entry->value);
        }
    }
}

VALUE hash_obj_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    std::stringstream stream;
    stream << "{";
    size_t count = 1;
    for (int32_t i = 0; i <= table->capacity; i++)
    {
        HashEntry *entry = &table->entries[i];
        if (entry->key != NIL_VAL)
        {
            call_function(vm, entry->key, common_strings[STRING_TO_STRING], 0, NULL);
            VALUE key_str = peek(vm, 0);
            call_function(vm, entry->value, common_strings[STRING_TO_STRING], 0, NULL);
            VALUE value_str = peek(vm, 0);
            stream << string_get_cstr(key_str) << ": " << string_get_cstr(value_str);
            if (count != table->count - 1)
                stream << ", ";
            count++;
            popMany(vm, 2);
        }
    }
    stream << "}";
    std::string result = stream.str();
    return copyString(vm, result.c_str(), result.length());
}

VALUE hash_values(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    VALUE result = list_create(vm);
    push(vm, result);
    for (int32_t i = 0; i <= table->capacity; i++)
    {
        HashEntry *entry = &table->entries[i];
        if (entry->key != NIL_VAL)
        {
            list_add(vm, result, 1, &entry->value);
        }
    }
    return pop(vm); // result
}

VALUE hash_keys(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    VALUE result = list_create(vm);
    push(vm, result);
    for (int32_t i = 0; i <= table->capacity; i++)
    {
        HashEntry *entry = &table->entries[i];
        if (entry->key != NIL_VAL)
        {
            list_add(vm, result, 1, &entry->key);
        }
    }
    return pop(vm); // result
}

void init_hash(VM *vm)
{
    hash_class = defineNativeClass(vm, "Hash", &hash_constructor, &hash_destructor, &hash_mark_contents, "Iterable", CLS_HASH, sizeof(HashTable), false);
    defineNativeMethod(vm, hash_class, &hash_add, "add", 2, false);
    defineNativeMethod(vm, hash_class, &hash_remove, "remove", 1, false);
    defineNativeMethod(vm, hash_class, &hash_iterable_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, hash_class, &hash_iterable_empty_q, "empty?", 0, false);
    defineNativeMethod(vm, hash_class, &hash_iterable_count, "count", 0, false);
    defineNativeMethod(vm, hash_class, &hash_iterable_iterator, "iterator", 0, false);
    defineNativeMethod(vm, hash_class, &hash_obj_to_string, "to_string", 0, false);
    defineNativeMethod(vm, hash_class, &hash_get, "get", 2, false);
    defineNativeMethod(vm, hash_class, &hash_has_key_q, "has_key?", 1, false);
    defineNativeMethod(vm, hash_class, &hash_values, "values", 0, false);
    defineNativeMethod(vm, hash_class, &hash_keys, "keys", 0, false);
    defineNativeOperator(vm, hash_class, &hash_find, 1, OPERATOR_INDEX);
    defineNativeOperator(vm, hash_class, &hash_add, 2, OPERATOR_INDEX_ASSIGN);

    hash_iterator_class = defineNativeClass(
        vm,
        "HashIterator",
        &hash_iterator_constructor,
        NULL,
        NULL,
        "Iterator",
        CLS_ITERATOR,
        sizeof(HashIterator),
        true);
    defineNativeMethod(vm, hash_iterator_class, &hash_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, hash_iterator_class, &hash_iterator_get_next, "get_next", 0, false);
    defineNativeMethod(vm, hash_iterator_class, &hash_iterator_to_string, "to_string", 0, false);
}

#ifdef __cplusplus
}
#endif
