#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mem.h"
#include "objects.h"
#include "scanner.h"
#include "comet.h"

static ObjClass *_string_class;

#define ALLOCATE_OBJ(vm, type, objectType) \
    (type *)allocateObject(vm, sizeof(type), objectType)

static Obj *allocateObject(VM *vm, size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm->objects;
    vm->objects = object;

#if DEBUG_LOG_GC
    printf("%p allocate %ld for %s\n", (void *)object, size, objTypeName(type));
#endif

    return object;
}

ObjBoundMethod *newBoundMethod(VM *vm, Value receiver, ObjClosure *method)
{
    ObjBoundMethod *bound = ALLOCATE_OBJ(vm, ObjBoundMethod, OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

static void init_class(ObjClass *klass, const char *name)
{
    size_t length = strlen(name) + 1;
    klass->name = ALLOCATE(char, length);
    klass->super_ = NULL;
    strncpy(klass->name, name, length);
    initTable(&klass->methods);
    initTable(&klass->staticMethods);
    for (int i = 0; i < NUM_OPERATORS; i++)
    {
        klass->operators[i] = NIL_VAL;
    }
}

ObjClass *newClass(VM *vm, const char *name)
{
    ObjClass *klass = ALLOCATE_OBJ(vm, ObjClass, OBJ_CLASS);
    init_class(klass, name);
    return klass;
}

ObjNativeClass *newNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor)
{
    ObjNativeClass *klass = ALLOCATE_OBJ(vm, ObjNativeClass, OBJ_NATIVE_CLASS);
    push(vm, OBJ_VAL(klass));
    init_class((ObjClass *)klass, name);
    klass->constructor = constructor;
    klass->destructor = destructor;
    return klass;
}

ObjClosure *newClosure(VM *vm, ObjFunction *function)
{
    ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++)
    {
        upvalues[i] = NULL;
    }
    ObjClosure *closure = ALLOCATE_OBJ(vm, ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction *newFunction(VM *vm)
{
    ObjFunction *function = ALLOCATE_OBJ(vm, ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NIL_VAL;
    initChunk(&function->chunk);
    return function;
}

Obj *newInstance(VM *vm, ObjClass *klass)
{
    if (OBJ_VAL(klass) == NIL_VAL)
    {
        fprintf(stderr, "Can't instantiate nil\n");
        abort();
        return NULL;
    }
    Obj *obj = (Obj *)klass;
    switch (obj->type)
    {
    case OBJ_CLASS:
    {
        ObjInstance *instance = ALLOCATE_OBJ(vm, ObjInstance, OBJ_INSTANCE);
        instance->klass = klass;
        initTable(&instance->fields);
        return (Obj *)instance;
    }
    case OBJ_NATIVE_CLASS:
    {
        ObjNativeInstance *instance = ALLOCATE_OBJ(vm, ObjNativeInstance, OBJ_NATIVE_INSTANCE);
        instance->instance.klass = klass;
        initTable(&instance->instance.fields);
        ObjNativeClass *native_klass = (ObjNativeClass *)klass;
        push(vm, OBJ_VAL(native_klass));
        if (native_klass->constructor != NULL)
        {
            instance->data = native_klass->constructor();
        }
        pop(vm);
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

ObjNative *newNativeFunction(VM *vm, NativeFn function)
{
    ObjNative *native = ALLOCATE_OBJ(vm, ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjNativeMethod *newNativeMethod(VM *vm, NativeMethod function, uint8_t arity, bool isStatic)
{
    ObjNativeMethod *method = ALLOCATE_OBJ(vm, ObjNativeMethod, OBJ_NATIVE_METHOD);
    method->function = function;
    method->arity = arity;
    method->isStatic = isStatic;
    return method;
}

static Value allocateString(VM *vm, char *chars, int length)
{
    ObjNativeInstance *string = (ObjNativeInstance *) newInstance(vm, _string_class);
    Value string_obj = OBJ_VAL(string);
    string->data = string_set_cstr(string, chars, length);
    push(vm, string_obj);
    internString(string_obj);
    pop(vm);
    return string_obj;
}

Value takeString(VM *vm, char *chars, int length)
{
    uint32_t hash = string_hash_cstr(chars, length);
    Value interned = findInternedString(chars, hash);
    if (interned != NIL_VAL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(vm, chars, length);
}

Value copyString(VM *vm, const char *chars, int length)
{
    uint32_t hash = string_hash_cstr(chars, length);
    Value interned = findInternedString(chars, hash);
    if (interned != NIL_VAL)
        return interned;

    char *copied_string = ALLOCATE(char, length + 1);
    memcpy(copied_string, chars, length);
    copied_string[length] = '\0';
    return allocateString(vm, copied_string, length);
}

ObjUpvalue *newUpvalue(VM *vm, Value *slot)
{
    ObjUpvalue *upvalue = ALLOCATE_OBJ(vm, ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

void registerStringClass(Value klass)
{
    _string_class = AS_CLASS(klass);
}

static void printFunction(ObjFunction *function)
{
    if (function->name == NIL_VAL)
    {
        printf("<script>");
        return;
    }
    printf("<fn %s>", string_get_cstr(function->name));
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
    case OBJ_CLASS:
    case OBJ_NATIVE_CLASS:
        printf("%s Class", AS_CLASS(value)->name);
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
    {
        if (strcmp(AS_INSTANCE(value)->klass->name, "String") == 0)
        {
            printf("\"%s\"", string_get_cstr(value));
        }
        else
        {
            printf("%s instance", AS_INSTANCE(value)->klass->name);
        }
        break;
    }
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
    case OBJ_UPVALUE:
        printf("upvalue");
        break;
    default:
        printf("Unknown object");
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
        case OPERATOR_INDEX_ASSIGN:
            return "[]=";
        case OPERATOR_MODULO:
            return "%";
        case NUM_OPERATORS:
            return "unknown";
        case OPERATOR_UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

OPERATOR getOperatorFromToken(TokenType token)
{
    if (token == TOKEN_STAR)
        return OPERATOR_MULTIPLICATION;
    else if (token == TOKEN_PLUS)
        return OPERATOR_PLUS;
    else if (token == TOKEN_MINUS)
        return OPERATOR_MINUS;
    else if (token == TOKEN_SLASH)
        return OPERATOR_DIVISION;
    else if (token == TOKEN_GREATER)
        return OPERATOR_GREATER_THAN;
    else if (token == TOKEN_GREATER_EQUAL)
        return OPERATOR_GREATER_EQUAL;
    else if (token == TOKEN_LESS)
        return OPERATOR_LESS_THAN;
    else if (token == TOKEN_LESS_EQUAL)
        return OPERATOR_LESS_EQUAL;
    else if (token == TOKEN_EQUAL_EQUAL)
        return OPERATOR_EQUALS;
    else if (token == TOKEN_LEFT_SQ_BRACKET)
        return OPERATOR_INDEX;
    else if (token == TOKEN_PERCENT)
        return OPERATOR_MODULO;

    return OPERATOR_UNKNOWN;
}
