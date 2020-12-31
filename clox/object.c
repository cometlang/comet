#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)

static Obj *allocateObject(size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

#if DEBUG_LOG_GC
    printf("%p allocate %ld for %s\n", (void *)object, size, objTypeName(type));
#endif

    return object;
}

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method)
{
    ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod,
                                         OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass *newClass(ObjString *name)
{
    ObjClass *klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    initTable(&klass->staticMethods);
    for (int i = 0; i < NUM_OPERATORS; i++)
    {
        klass->operators[i] = NIL_VAL;
    }
    return klass;
}

ObjNativeClass *newNativeClass(ObjString *name, NativeConstructor constructor, NativeDestructor destructor)
{
    ObjNativeClass *klass = ALLOCATE_OBJ(ObjNativeClass, OBJ_NATIVE_CLASS);
    klass->klass.name = name;
    initTable(&klass->klass.methods);
    initTable(&klass->klass.staticMethods);
    klass->constructor = constructor;
    klass->destructor = destructor;
    return klass;
}

ObjClosure *newClosure(ObjFunction *function)
{
    ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++)
    {
        upvalues[i] = NULL;
    }
    ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction *newFunction()
{
    ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

Obj *newInstance(ObjClass *klass)
{
    Obj *obj = (Obj *)klass;
    switch (obj->type)
    {
    case OBJ_CLASS:
    {
        ObjInstance *instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
        instance->klass = klass;
        initTable(&instance->fields);
        return (Obj *)instance;
    }
    case OBJ_NATIVE_CLASS:
    {
        ObjNativeInstance *instance = ALLOCATE_OBJ(ObjNativeInstance, OBJ_NATIVE_INSTANCE);
        instance->instance.klass = klass;
        initTable(&instance->instance.fields);
        ObjNativeClass *native_klass = (ObjNativeClass *)klass;
        if (native_klass->constructor != NULL)
        {
            instance->data = native_klass->constructor();
        }
        return (Obj *)instance;
    }
    default:
    {
        fprintf(stderr, "Can't instantiate something that isn't a class\n");
        abort();
        return NULL;
    }
    }
}

ObjNative *newNative(NativeFn function)
{
    ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjNativeMethod *newNativeMethod(Value receiver, NativeMethod function, bool isStatic)
{
    ObjNativeMethod *method = ALLOCATE_OBJ(ObjNativeMethod, OBJ_NATIVE_METHOD);
    method->receiver = receiver;
    method->function = function;
    method->isStatic = isStatic;
    return method;
}

static ObjString *allocateString(char *chars, int length,
                                 uint32_t hash)
{
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    internString(string);
    pop();

    return string;
}

static uint32_t hashString(const char *key, int length)
{
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++)
    {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString *takeString(char *chars, int length)
{
    uint32_t hash = hashString(chars, length);
    ObjString *interned = findInternedString(chars, length, hash);
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(chars, length, hash);
}

ObjString *copyString(const char *chars, int length)
{
    uint32_t hash = hashString(chars, length);
    ObjString *interned = findInternedString(chars, length, hash);
    if (interned != NULL)
        return interned;
    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

ObjUpvalue *newUpvalue(Value *slot)
{
    ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

static void printFunction(ObjFunction *function)
{
    if (function->name == NULL)
    {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
    case OBJ_CLASS:
    case OBJ_NATIVE_CLASS:
        printf("%s Class", AS_CLASS(value)->name->chars);
        break;
    case OBJ_NATIVE_METHOD:
        printf("<native method>");
        break;
    case OBJ_BOUND_METHOD:
        printFunction(AS_BOUND_METHOD(value)->method->function);
        break;
    case OBJ_CLOSURE:
        printFunction(AS_CLOSURE(value)->function);
        break;
    case OBJ_FUNCTION:
        printFunction(AS_FUNCTION(value));
        break;
    case OBJ_INSTANCE:
    case OBJ_NATIVE_INSTANCE:
        // obj.to_string();
        printf("%s instance", AS_INSTANCE(value)->klass->name->chars);
        break;
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    case OBJ_UPVALUE:
        printf("upvalue");
        break;
    }
}

const char *objTypeName(ObjType type)
{
    switch (type)
    {
    case OBJ_BOUND_METHOD:
        return "method";
    case OBJ_CLASS:
        return "class";
    case OBJ_NATIVE_CLASS:
        return "native class";
    case OBJ_NATIVE_METHOD:
        return "native method";
    case OBJ_CLOSURE:
        return "closure";
    case OBJ_FUNCTION:
        return "function";
    case OBJ_INSTANCE:
        return "instance";
    case OBJ_NATIVE_INSTANCE:
        return "native instance";
    case OBJ_NATIVE:
        return "native function";
    case OBJ_STRING:
        return "string";
    case OBJ_UPVALUE:
        return "upvalue";
    }

    return "unknown";
}

const char *getOperatorString(OPERATOR operator)
{
    switch (operator)
    {
        case OPERATOR_MULTIPLICATION:
            return "*";
        case OPERATOR_PLUS:
            return "+";
        case OPERATOR_MINUS:
            return "-";
        case OPERATOR_DIVISION:
            return "/";
        case OPERATOR_GREATER_THAN:
            return ">";
        case OPERATOR_LESS_THAN:
            return "<";
        case OPERATOR_GREATER_EQUAL:
            return ">=";
        case OPERATOR_LESS_EQUAL:
            return "<=";
        case OPERATOR_EQUALS:
            return "==";
        case OPERATOR_INDEX:
            return "[]";
        case NUM_OPERATORS:
            return "unknown";
    }
    return "unknown";
}
