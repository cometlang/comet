#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct sObj Obj;

#if NAN_TAGGING

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)

// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)

// Tag values for the different singleton values.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE  3 // 11.

typedef uint64_t Value;

#define IS_BOOL(v)    (((v) & FALSE_VAL) == FALSE_VAL)
#define IS_NIL(v)     ((v) == NIL_VAL)
// If the NaN bits are set, it's not a number.
#define IS_NUMBER(v)  (((v) & QNAN) != QNAN)
#define IS_OBJ(v)     (((v) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(v)    ((v) == TRUE_VAL)
#define AS_NUMBER(v)  valueToNum(v)
#define AS_OBJ(v)     ((Obj*)(uintptr_t)((v) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(boolean) ((boolean) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL         ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL          ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NUMBER_VAL(num)   numToValue(num)
// The triple casting is necessary here to satisfy some compilers:
// 1. (uintptr_t) Convert the pointer to a number of the right size.
// 2. (uint64_t)  Pad it up to 64 bits in 32-bit builds.
// 3. Or in the bits to make a tagged Nan.
// 4. Cast to a typedef'd value.
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

// A union to let us reinterpret a double as raw bits and back.
typedef union {
  uint64_t bits64;
  uint32_t bits32[2];
  double num;
} DoubleUnion;

static inline double valueToNum(Value value) {
  DoubleUnion data;
  data.bits64 = value;
  return data.num;
}

static inline Value numToValue(double num) {
  DoubleUnion data;
  data.num = num;
  return data.bits64;
}

#else

typedef enum
{
    VAL_BOOL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef struct
{
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value)     ((value).as.obj)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define TRUE_VAL BOOL_VAL(true)
#define FALSE_VAL BOOL_VAL(false)
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)   ((Value){ VAL_OBJ, { .obj = (Obj*)object } })

#endif

#define VALUE Value

#define NIL_VAL (OBJ_VAL(&nil_instance))

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
void printValue(Value value);

#endif
