#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mem.h"
#include "objects.h"
#include "scanner.h"
#include "comet.h"

#define ALLOCATE_OBJ(vm, type, objectType) \
    (type *)allocateObject(vm, sizeof(type), objectType)

ObjBoundMethod *newBoundMethod(VM *vm, Value receiver, ObjClosure *method)
{
    ObjBoundMethod *bound = ALLOCATE_OBJ(vm, ObjBoundMethod, OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

static void init_class(ObjClass *klass, const char *name, ClassType classType, bool final)
{
    size_t length = strlen(name) + 1;
    klass->name = ALLOCATE(char, length);
    klass->super_ = NULL;
    klass->classType = classType;
    klass->final = final;
    strncpy(klass->name, name, length);
    initTable(&klass->methods);
    initTable(&klass->staticMethods);
    for (int i = 0; i < NUM_OPERATORS; i++)
    {
        klass->operators[i] = NIL_VAL;
    }
}

ObjClass *newClass(VM *vm, const char *name, ClassType classType, bool final)
{
    ObjClass *klass = ALLOCATE_OBJ(vm, ObjClass, OBJ_CLASS);
    init_class(klass, name, classType, final);
    return klass;
}

ObjNativeClass *newNativeClass(
    VM *vm,
    const char *name,
    NativeConstructor constructor,
    NativeDestructor destructor,
    MarkNativeObject marker,
    ClassType classType,
    size_t allocSize,
    bool final)
{
    ObjNativeClass *klass = ALLOCATE_OBJ(vm, ObjNativeClass, OBJ_NATIVE_CLASS);
    init_class((ObjClass *)klass, name, classType, final);
    klass->constructor = constructor;
    klass->destructor = destructor;
    klass->marker = marker;
    klass->allocSize = allocSize == 0 ? sizeof(ObjInstance) : allocSize;
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

ObjFunction *newFunction(VM *vm, const char *filename, VALUE module)
{
    ObjFunction *function = ALLOCATE_OBJ(vm, ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NIL_VAL;
    function->optionalArgCount = 0;
    function->module = module;
    initChunk(&function->chunk, filename);
    return function;
}

Obj *newInstance(VM *vm, ObjClass *klass)
{
    if (klass->classType == CLS_NIL || klass->classType == CLS_BOOLEAN)
    {
        runtimeError(vm, "Can't instantiate %s\n", klass->name);
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
        return AS_OBJ(pop(vm));
    }
    case OBJ_NATIVE_CLASS:
    {
        ObjNativeClass *native_klass = (ObjNativeClass *)klass;
        ObjInstance *instance = (ObjInstance *)allocateObject(vm, native_klass->allocSize, OBJ_NATIVE_INSTANCE);
        instance->klass = klass;
        initTable(&instance->fields);
        push(vm, OBJ_VAL(native_klass));
        if (native_klass->constructor != NULL)
        {
            native_klass->constructor(instance);
        }
        pop(vm);
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

ObjNativeMethod *newNativeMethod(VM *vm, NativeMethod function, uint8_t arity, bool isStatic, Value name)
{
    ObjNativeMethod *method = ALLOCATE_OBJ(vm, ObjNativeMethod, OBJ_NATIVE_METHOD);
    method->function = function;
    method->arity = arity;
    method->isStatic = isStatic;
    method->name = name;
    return method;
}

static Value allocateString(VM *vm, char *chars, int length)
{
    VALUE string_obj = string_create(vm, chars, length);
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

Value copyString(VM *vm, const char *chars, size_t length)
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
    if (IS_NUMBER(value))
    {
        printf("%.17g", number_get_value(value));
        return;
    }
    switch (OBJ_TYPE(value))
    {
    case OBJ_CLASS:
    case OBJ_NATIVE_CLASS:
        printf("%s Class", AS_CLASS(value)->name);
        break;
    case OBJ_NATIVE_METHOD:
        printf("<native method %s>", string_get_cstr(AS_NATIVE_METHOD(value)->name));
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
        if (IS_INSTANCE_OF_STDLIB_TYPE(value, CLS_STRING))
        {
            printf("\"%s\"", string_get_cstr(value));
        }
        else if (IS_INSTANCE_OF_STDLIB_TYPE(value, CLS_BOOLEAN))
        {
            if (bool_is_falsey(value))
                printf("false");
            else
                printf("true");
        }
        else if (IS_INSTANCE_OF_STDLIB_TYPE(value, CLS_MODULE))
        {
            printf("Module %s", module_filename(value));
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

const char* getClassNameFromInstance(VALUE instance)
{
    return AS_INSTANCE(instance)->klass->name;
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
        case OPERATOR_BITWISE_AND:
            return "&";
        case OPERATOR_BITWISE_OR:
            return "|";
        case OPERATOR_BITWISE_XOR:
            return "^";
        case OPERATOR_BITSHIFT_LEFT:
            return "<<";
        case OPERATOR_BITSHIFT_RIGHT:
            return ">>";
        case OPERATOR_BITWISE_NEGATE:
            return "~";
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

OPERATOR getOperatorFromToken(TokenType_t token)
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
