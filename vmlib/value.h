#ifndef clox_value_h
#define clox_value_h
#include <string.h>
#include "common.h"

typedef struct sObj Obj;

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)

// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)

typedef uint64_t Value;

// The triple casting is necessary here to satisfy some compilers:
// 1. (uintptr_t) Convert the pointer to a number of the right size.
// 2. (uint64_t)  Pad it up to 64 bits in 32-bit builds.
// 3. Or in the bits to make a tagged Nan.
// 4. Cast to a typedef'd value.
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

#define VALUE Value

#define NIL_VAL (OBJ_VAL(&nil_instance))
#define TRUE_VAL (OBJ_VAL(boolean_true))
#define FALSE_VAL (OBJ_VAL(boolean_false))

#define NUMBER_VAL(num)  numToValue(num)
#define AS_NUMBER(value) valueToNum(value)

#define IS_NIL(v)        ((v) == NIL_VAL)
#define IS_OBJ(v)        (((v) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))
#define AS_OBJ(v)        ((Obj*)(uintptr_t)((v) & ~(SIGN_BIT | QNAN)))
#define IS_NUMBER(value) (((value) & QNAN) != QNAN)

typedef struct _vm VM;

typedef struct
{
    int capacity;
    int count;
    Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printObject(Value value);

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}


#endif
