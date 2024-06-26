#ifndef _COMET_OBJECT_DEFS_H_
#define _COMET_OBJECT_DEFS_H_

#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_NATIVE_CLASS(value) isObjType(value, OBJ_NATIVE_CLASS)
#define IS_NATIVE_METHOD(value) isObjType(value, OBJ_NATIVE_METHOD)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE_INSTANCE(value) isObjType(value, OBJ_NATIVE_INSTANCE)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

#define AS_BOUND_METHOD(value) ((ObjBoundMethod *)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))
#define AS_NATIVE_CLASS(value) ((ObjNativeClass *)AS_OBJ(value))
#define AS_NATIVE_METHOD(value) ((ObjNativeMethod *)AS_OBJ(value))
#define AS_UPVALUE(value) ((ObjUpvalue *)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE_OBJ(value) (((ObjNative *)AS_OBJ(value)))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)

#define IS_INSTANCE_OF_STDLIB_TYPE(value, classType) isObjOfStdlibClassType(value, classType)

typedef enum
{
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_NATIVE_CLASS,
    OBJ_NATIVE_METHOD,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE_INSTANCE,
    OBJ_NATIVE,
    OBJ_UPVALUE,
} ObjType;

typedef enum
{
    CLS_BOOLEAN,
    CLS_COND_VAR,
    CLS_COLOUR,
    CLS_DATETIME,
    CLS_DIRECTORY,
    CLS_DURATION,
    CLS_ENUM,
    CLS_ENUM_VALUE,
    CLS_ENV,
    CLS_EXCEPTION,
    CLS_FILE,
    CLS_FUNCTION,
    CLS_ITERABLE,
    CLS_ITERATOR,
    CLS_LIST,
    CLS_HASH,
    CLS_IMAGE,
    CLS_MODULE,
    CLS_MUTEX,
    CLS_NIL,
    CLS_NUMBER,
    CLS_OBJECT,
    CLS_PROCESS,
    CLS_PROCESS_RUN_RESULT,
    CLS_SET,
    CLS_SOCKET,
    CLS_STRING,
    CLS_STRING_BUILDER,
    CLS_THREAD,
    CLS_USER_DEF,
} ClassType;

typedef enum
{
    OPERATOR_MULTIPLICATION,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_DIVISION,
    OPERATOR_MODULO,
    OPERATOR_GREATER_THAN,
    OPERATOR_LESS_THAN,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_LESS_EQUAL,
    OPERATOR_EQUALS,
    OPERATOR_INDEX,
    OPERATOR_INDEX_ASSIGN,
    OPERATOR_BITWISE_OR,
    OPERATOR_BITWISE_AND,
    OPERATOR_BITWISE_XOR,
    OPERATOR_BITWISE_NEGATE,
    OPERATOR_BITSHIFT_LEFT,
    OPERATOR_BITSHIFT_RIGHT,
    NUM_OPERATORS,
    OPERATOR_UNKNOWN,
} OPERATOR;

struct sObj
{
    ObjType type;
#if REF_COUNT_MEM_MANAGEMENT
    int8_t refCount;
#else
    bool isMarked;
#endif
    struct sObj *next;
};

typedef struct
{
    Obj obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    Value name;
    Value module;
    uint16_t optionalArguments[MAX_ARGS];
    uint8_t optionalArgCount;
    Value *attributes;
    int attributeCount;
    bool restParam;
} ObjFunction;

typedef Value (*NativeFn)(VM *vm, int argCount, Value *args);

typedef struct
{
    Obj obj;
    NativeFn function;
} ObjNative;

typedef struct sUpvalue
{
    Obj obj;
    Value *location;
    Value closed;
    struct sUpvalue *next;
} ObjUpvalue;

typedef struct
{
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct sObjClass
{
    Obj obj;
    char *name;
    Table methods;
    Table staticMethods;
    Value operators[NUM_OPERATORS];
    struct sObjClass *super_;
    ClassType classType;
    bool final;
    Value *attributes;
    int attributeCount;
} ObjClass;

typedef void (*NativeConstructor)(void *data);
typedef void(*NativeDestructor)(void *data);
typedef Value (*NativeMethod)(VM *vm, Value receiver, int argCount, Value *args);
typedef void (*MarkNativeObject)(Value self);

typedef struct sNativeClass
{
    ObjClass klass;
    NativeConstructor constructor;
    NativeDestructor destructor;
    MarkNativeObject marker;
    size_t allocSize;
} ObjNativeClass;

typedef struct sNativeMethod
{
    Obj obj;
    NativeMethod function;
    uint8_t arity;
    bool isStatic;
    Value name;
} ObjNativeMethod;

typedef struct
{
    Obj obj;
    ObjClass *klass;
    Table fields;
} ObjInstance;

typedef struct
{
    Obj obj;
    Value receiver;
    ObjClosure *method;
} ObjBoundMethod;

extern ObjInstance nil_instance;
extern ObjInstance *boolean_true;
extern ObjInstance *boolean_false;

#endif