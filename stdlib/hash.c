#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

#include <stdlib.h>

#define TABLE_MAX_LOAD 0.75

typedef struct hash_entry
{
    VALUE key;
    VALUE value;
} HashEntry;

typedef struct
{
    int count;
    size_t capacity;
    HashEntry *entries;
} HashTable;

typedef struct {
    HashEntry **entries;
    int count;
} HashTableIterator;

VALUE hash_iterable_contains_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_iterable_empty_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE hash_iterable_iterator(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    // This needs to be a HashIterator class and return an instance of that.
    // Ideally this would be done as a generator method, instead of always
    // gathering all the values, but I don't have generator methods and don't
    // know how to implement them.
    HashTable *data = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    HashTableIterator *iterator = ALLOCATE(HashTableIterator, 1);
    size_t index = 0;
    iterator->entries = ALLOCATE(HashEntry*, data->count);
    for (size_t i = 0; i < data->capacity; i++)
    {
        if (data->entries[i].key != NIL_VAL)
        {
            HashEntry *current = &data->entries[i];
        }
    }
    return OBJ_VAL(iterator);
}

void *hash_constructor(void)
{
    Table *table = ALLOCATE(Table, 1);
    table->count = 0;
    table->capacity = -1;
    table->entries = NULL;
    return table;
}

void hash_destructor(void *data)
{
    HashTable *table = (HashTable *)data;
    FREE_ARRAY(HashEntry, table->entries, table->capacity + 1);
    FREE(HashTable, table);
}

static HashEntry *find_entry(HashEntry *entries, int capacity,
                             ObjString *key)
{
    uint32_t index = key->hash & capacity;
    HashEntry *tombstone = NULL;

    for (;;)
    {
        HashEntry *entry = &entries[index];

        if (entry->key == NULL)
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
        else if (entry->key == key)
        {
            // We found the key.
            return entry;
        }

        index = (index + 1) & capacity;
    }
}

VALUE hash_get(VALUE self, int arg_count, VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count == 0)
        return false;

    VALUE key = arguments[0];
    HashEntry *entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return NIL_VAL;  // throw an exception

    return entry->value;
}

static void adjustCapacity(HashTable *table, int capacity)
{
    HashEntry *entries = ALLOCATE(HashEntry, capacity + 1);
    for (int i = 0; i <= capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i <= table->capacity; i++)
    {
        HashEntry *entry = &table->entries[i];
        if (entry->key == NULL)
            continue;

        HashEntry *dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    FREE_ARRAY(HashEntry, table->entries, table->capacity + 1);

    table->entries = entries;
    table->capacity = capacity;
}

VALUE hash_add(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count + 1 > (table->capacity + 1) * TABLE_MAX_LOAD)
    {
        // Figure out the new table size.
        int capacity = GROW_CAPACITY(table->capacity + 1) - 1;
        adjustCapacity(table, capacity);
    }

    VALUE key = arguments[0];
    VALUE value = arguments[1];
    Entry *entry = find_entry(table->entries, table->capacity, key);

    bool isNewKey = entry->key == NULL;
    if (isNewKey && IS_NIL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

VALUE hash_remove(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    HashTable *table = GET_NATIVE_INSTANCE_DATA(HashTable, self);
    if (table->count == 0)
        return false;

    // Find the entry.
    VALUE key = arguments[0];
    HashEntry *entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = TRUE_VAL;

    return true;
}

void tableAddAll(Table *from, Table *to)
{
    for (int i = 0; i <= from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL)
        {
            tableSet(to, entry->key, entry->value);
        }
    }
}

void tableRemoveWhite(Table *table)
{
    for (int i = 0; i <= table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked)
        {
            tableDelete(table, entry->key);
        }
    }
}

void markTable(Table *table)
{
    for (int i = 0; i <= table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        markObject((Obj *)entry->key);
        markValue(entry->value);
    }
}

VALUE hash_obj_to_string(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{

    // for (int i = 0; i <= table->capacity; i++)
//     {
//         Entry *entry = &table->entries[i];
//         if (entry->key != NULL)
//         {
//             printValue(OBJ_VAL(entry->key));
//             printf("\n");
//         }
//     }
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
    defineNativeOperator(klass, &hash_add, OPERATOR_INDEX_ASSIGN);
}
