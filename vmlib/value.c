#include <stdio.h>
#include <string.h>

#include "object.h"
#include "mem.h"
#include "value.h"

void initValueArray(ValueArray *array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray *array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(array->values, Value, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray *array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value)
{
#if NAN_TAGGING
    if (IS_BOOL(value))
    {
        printf(AS_BOOL(value) ? "true" : "false");
    }
    else if (IS_NIL(value))
    {
        printf("nil");
    }
    else if (IS_NUMBER(value))
    {
        printf("%g", AS_NUMBER(value));
    }
    else if (IS_OBJ(value))
    {
        printObject(value);
    }
#else
    switch (value.type)
    {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil");
        break;
    case VAL_NUMBER:
        printf("%g", AS_NUMBER(value));
        break;
    case VAL_OBJ:
        printObject(value);
        break;
    }
#endif
}

bool valuesEqual(Value a, Value b)
{
#if NAN_TAGGING
    return a == b;
#else
    if (a.type != b.type)
        return false;

    switch (a.type)
    {
    case VAL_BOOL:
        return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
        return true;
    case VAL_NUMBER:
        return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:
        return AS_OBJ(a) == AS_OBJ(b);
    }

    return false;
#endif
}
