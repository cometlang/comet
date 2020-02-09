#ifndef clox_object_h
#define clox_object_h

#include "common.h"
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
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_BOUND_METHOD(value) ((ObjBoundMethod *)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))
#define AS_NATIVE_CLASS(value) ((ObjNativeClass *)AS_OBJ(value))
#define AS_NATIVE_METHOD(value) ((ObjNativeMethod *)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE_INSTANCE(value) ((ObjNativeInstance *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

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
    OBJ_STRING,
    OBJ_UPVALUE,
} ObjType;

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
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct
{
    Obj obj;
    NativeFn function;
} ObjNative;

struct sObjString
{
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

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
    ObjString *name;
    Table methods;
} ObjClass;

typedef void *(NativeConstructor)(void);
typedef void(NativeDestructor)(void *);

typedef struct sNativeClass
{
    ObjClass klass;
    NativeConstructor *constructor;
    NativeDestructor *destructor;
} ObjNativeClass;

typedef struct sNativeMethod
{
    Obj obj;
    Value receiver;
    NativeFn function;
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
ObjClass *newClass(ObjString *name);
ObjNativeClass *newNativeClass(ObjString *name, NativeConstructor *constructor, NativeDestructor *destructor);
ObjNativeMethod *newNativeMethod(Value receiver, NativeFn function);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
Obj *newInstance(ObjClass *klass);
ObjNative *newNative(NativeFn function);
ObjString *takeString(char *chars, int length);
ObjString *copyString(const char *chars, int length);
ObjUpvalue *newUpvalue(Value *slot);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
