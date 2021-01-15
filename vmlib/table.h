#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct
{
    Value key;
    Value value;
} Entry;

typedef struct
{
    int count;
    int capacity;
    Entry *entries;
} Table;

void initTable(Table *table);
void freeTable(Table *table);
bool tableGet(VM *vm, Table *table, Value key, Value *result);
bool tableSet(VM *vm, Table *table, Value key, Value value);
bool tableDelete(VM *vm, Table *table, Value key);
void tableAddAll(VM *vm, Table *from, Table *to);
Value tableFindString(VM *vm, Table *table, const char *chars, uint32_t hash);
void tableRemoveWhite(VM *vm, Table *table);
void markTable(VM *vm, Table *table);

void tablePrintKeys(Table *table);

#endif