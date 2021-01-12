#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "scanner.h"
#include "chunk.h"
#include "value.h"
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
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE_INSTANCE(value) ((ObjNativeInstance *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)

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
    OPERATOR_MULTIPLICATION,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_DIVISION,
    OPERATOR_GREATER_THAN,
    OPERATOR_LESS_THAN,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_LESS_EQUAL,
    OPERATOR_EQUALS,
    OPERATOR_INDEX,
    OPERATOR_INDEX_ASSIGN,
    NUM_OPERATORS,
    OPERATOR_UNKNOWN,
} OPERATOR;

struct sObj
{
    ObjType type;
    bool isMarked;
    struct sObj *next;
};

typedef struct
{
    Obj obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    Value name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

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
} ObjClass;

typedef void *(*NativeConstructor)(void);
typedef void(*NativeDestructor)(void *data);
typedef Value (*NativeMethod)(Value receiver, int argCount, Value *args);

typedef struct sNativeClass
{
    ObjClass klass;
    NativeConstructor constructor;
    NativeDestructor destructor;
} ObjNativeClass;

typedef struct sNativeMethod
{
    Obj obj;
    Value receiver;
    NativeMethod function;
    bool isStatic;
} ObjNativeMethod;

typedef struct
{
    Obj obj;
    ObjClass *klass;
    Table fields;
} ObjInstance;

typedef struct
{
    ObjInstance instance;
    void *data;
} ObjNativeInstance;

typedef struct
{
    Obj obj;
    Value receiver;
    ObjClosure *method;
} ObjBoundMethod;

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method);
ObjClass *newClass(const char *name);
ObjNativeClass *newNativeClass(const char *name, NativeConstructor constructor, NativeDestructor destructor);
ObjNativeMethod *newNativeMethod(Value receiver, NativeMethod function, bool isStatic);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
Obj *newInstance(ObjClass *klass);
ObjNative *newNativeFunction(NativeFn function);
Value takeString(char *chars, int length);
Value copyString(const char *chars, int length);
ObjUpvalue *newUpvalue(Value *slot);
void printObject(Value value);
const char *objTypeName(ObjType type);
const char *getOperatorString(OPERATOR operator);
OPERATOR getOperatorFromToken(TokenType token);

void registerStringClass(Value klass);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
