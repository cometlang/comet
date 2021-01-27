#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mem.h"
#include "objects.h"
#include "table.h"
#include "value.h"
#include "comet.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table)
{
    table->count = 0;
    table->capacity = -1;
    table->entries = NULL;
}

void freeTable(Table *table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity + 1);
    initTable(table);
}

static Entry *findEntry(VM UNUSED(*vm), Entry *entries, int capacity,
                        Value key)
{
    const char *key_string = string_get_cstr(key);
    int key_str_len = strlen(key_string);
    uint32_t hash = (uint32_t) string_hash_cstr(key_string, key_str_len);
    uint32_t index = hash & capacity;
    Entry *tombstone = NULL;

    for (;;)
    {
        Entry *entry = &entries[index];

        if (entry->key == NIL_VAL)
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
        else if (strcmp(string_get_cstr(entry->key), key_string) == 0)
        {
            // We found the key.
            return entry;
        }

        index = (index + 1) & capacity;
    }
}

bool tableGet(VM *vm, Table *table, Value key, Value *result)
{
    if (table->count == 0)
        return false;

    Entry *entry = findEntry(vm, table->entries, table->capacity, key);
    if (entry->key == NIL_VAL)
        return false;

    *result = entry->value;
    return true;
}

static void adjustCapacity(VM *vm, Table *table, int capacity)
{
    Entry *entries = ALLOCATE(Entry, capacity + 1);
    for (int i = 0; i <= capacity; i++)
    {
        entries[i].key = NIL_VAL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i <= table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry == NULL || entry->key == NIL_VAL)
            continue;

        Entry *dest = findEntry(vm, entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    FREE_ARRAY(Entry, table->entries, table->capacity + 1);

    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(VM *vm, Table *table, Value key, Value value)
{
    if (table->count + 1 > (table->capacity + 1) * TABLE_MAX_LOAD)
    {
        // Figure out the new table size.
        int capacity = GROW_CAPACITY(table->capacity + 1) - 1;
        adjustCapacity(vm, table, capacity);
    }

    Entry *entry = findEntry(vm, table->entries, table->capacity, key);

    bool isNewKey = entry->key == NIL_VAL;
    if (isNewKey && IS_NIL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(VM *vm, Table *table, Value key)
{
    if (table->count == 0)
        return false;

    // Find the entry.
    Entry *entry = findEntry(vm, table->entries, table->capacity, key);
    if (entry != NULL && entry->key == NIL_VAL)
        return false;

    // Place a tombstone in the entry.
    entry->key = NIL_VAL;
    entry->value = TRUE_VAL;

    return true;
}

void tableAddAll(VM *vm, Table *from, Table *to)
{
    for (int i = 0; i <= from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry != NULL && entry->key != NIL_VAL)
        {
            tableSet(vm, to, entry->key, entry->value);
        }
    }
}

Value tableFindString(VM *vm, Table *table, const char *chars, uint32_t hash)
{
    if (table->count == 0)
        return NIL_VAL;

    uint32_t index = hash & table->capacity;

    for (;;)
    {
        Entry *entry = &table->entries[index];

        if (entry->key == NIL_VAL)
        {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NIL(entry->value))
                return NIL_VAL;
        }
        else
        {
            uint32_t entry_hash = (uint32_t) number_get_value(string_hash(vm, entry->key, 0, NULL));
            if (entry_hash == hash &&
                string_compare_to_cstr(entry->key, chars) == 0)
            {
                // We found it.
                return entry->key;
            }
        }

        index = (index + 1) & table->capacity;
    }
}

void tablePrintKeys(Table *table)
{
    for (int i = 0; i <= table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key != NIL_VAL)
        {
            printObject(OBJ_VAL(entry->key));
            printf("\n");
        }
    }
}


void tableRemoveWhite(VM *vm, Table *table)
{
    for (int i = 0; i <= table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entry->key != NIL_VAL && !AS_OBJ(entry->key)->isMarked)
        {
            tableDelete(vm, table, entry->key);
        }
    }
}

void markTable(VM *vm, Table *table)
{
    for (int i = 0; i <= table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        markValue(vm, entry->key);
        markValue(vm, entry->value);
    }
}
