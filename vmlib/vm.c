#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vm.h"
#include "compiler.h"
#include "debug.h"
#include "mem.h"

#include "comet.h"

__thread VM vm;
static Table globals;
static Table strings;

void markGlobals(void)
{
    markTable(&globals);
    markTable(&strings);
    markObject((Obj *)vm.initString);
}

void removeWhiteStrings(void)
{
    tableRemoveWhite(&strings);
}

Value findInternedString(const char *chars, uint32_t hash)
{
    return tableFindString(&strings, chars, hash);
}

bool internString(Value string)
{
    return tableSet(&strings, string, NIL_VAL);
}

bool findGlobal(Value name, Value *value)
{
    return tableGet(&globals, name, value);
}

bool addGlobal(Value name, Value value)
{
    DEBUG_ASSERT(strcmp(get_cstr(AS_INSTANCE(name)->klass->name), "String") == 0);
    return tableSet(&globals, name, value);
}

static void resetStack(void)
{
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

static Value getStackTrace(void)
{
#define MAX_LINE_LENGTH 1024
    char *stacktrace = ALLOCATE(char, vm.frameCount * MAX_LINE_LENGTH);
    uint16_t index = 0;
    for (int i = vm.frameCount - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm.frames[i];
        ObjFunction *function = frame->closure->function;
        // -1 because the IP is sitting on the next instruction to be
        // executed.
        size_t instruction = frame->ip - function->chunk.code - 1;
        uint32_t lineno = function->chunk.lines[instruction];
        index += snprintf(
            &stacktrace[index],
            MAX_LINE_LENGTH,
            "%s:%d - %s()\n",
            function->chunk.filename,
            lineno,
            function->name == NIL_VAL ? "script" : string_get_cstr(function->name));
    }
    Value result = copyString(stacktrace, index);
    FREE_ARRAY(char, stacktrace, vm.frameCount * MAX_LINE_LENGTH);
    return result;
#undef MAX_LINE_LENGTH
}

void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    fprintf(stderr, "%s", string_get_cstr(getStackTrace()));
    resetStack();
}

void initVM(void)
{
    resetStack();
    vm.objects = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    initTable(&globals);
    initTable(&strings);

    vm.initString = copyString("init", 4);

    init_stdlib();
}

void freeVM(void)
{
    freeTable(&globals);
    freeTable(&strings);
    vm.initString = NIL_VAL;
    freeObjects();
}

void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(void)
{
    vm.stackTop--;
    return *vm.stackTop;
}

Value popMany(int count)
{
    vm.stackTop -= count;
    return *vm.stackTop;
}

Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure *closure, int argCount)
{
    if (argCount != closure->function->arity)
    {
        runtimeError("Expected %d arguments but got %d.",
                     closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX)
    {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount)
{
    if (IS_OBJ(callee))
    {
        switch (OBJ_TYPE(callee))
        {
        case OBJ_BOUND_METHOD:
        {
            ObjBoundMethod *bound = AS_BOUND_METHOD(callee);

            // Replace the bound method with the receiver so it's in the
            // right slot when the method is called.
            vm.stackTop[-argCount - 1] = bound->receiver;
            return call(bound->method, argCount);
        }
        case OBJ_NATIVE_CLASS:
        case OBJ_CLASS:
        {
            ObjClass *klass = AS_CLASS(callee);
            Value instance = OBJ_VAL(newInstance(klass));
            vm.stackTop[-argCount - 1] = instance;
            // Call the initializer, if there is one.
            Value initializer;
            if (tableGet(&klass->methods, vm.initString, &initializer))
            {
                if (IS_NATIVE_METHOD(initializer))
                {
                    AS_NATIVE_METHOD(initializer)->function(instance, argCount, vm.stackTop - argCount);
                    popMany(argCount);
                    return true;
                }
                else
                {
                    return call(AS_CLOSURE(initializer), argCount);
                }
            }
            else if (argCount != 0)
            {
                runtimeError("Expected 0 arguments but got %d.", argCount);
                return false;
            }
            return true;
        }
        case OBJ_CLOSURE:
            return call(AS_CLOSURE(callee), argCount);

        case OBJ_NATIVE:
        {
            NativeFn native = AS_NATIVE(callee);
            Value result = native(argCount, vm.stackTop - argCount);
            popMany(argCount + 1);
            push(result);
            return true;
        }

        default:
            printObject(callee);
            printf("\nObj type: %s\n", objTypeName(OBJ_TYPE(callee)));
            // Non-callable object type.
            break;
        }
    }

    runtimeError("Can only call functions and classes.");
    return false;
}

static bool callNativeMethod(Value receiver, ObjNativeMethod *method, int argCount)
{
    Value result = method->function(receiver, argCount, vm.stackTop - argCount);
    popMany(argCount + 1);
    push(result);
    return true;
}

static Value findMethod(ObjClass *klass, Value name)
{
    Value result;
    if (tableGet(&klass->methods, name, &result))
    {
        return result;
    }
    return NIL_VAL;
}

static Value findStaticMethod(ObjClass *klass, Value name)
{
    Value result;
    if (tableGet(&klass->staticMethods, name, &result))
    {
        return result;
    }
    return NIL_VAL;
}

static bool invokeFromClass(ObjClass *klass, Value name,
                            int argCount)
{
    Value method = findMethod(klass, name);
    if (IS_BOUND_METHOD(method) || IS_CLOSURE(method))
    {
        return call(AS_CLOSURE(method), argCount);
    }

    method = findStaticMethod(klass, name);
    if (IS_NATIVE_METHOD(method) && AS_NATIVE_METHOD(method)->isStatic)
    {
        return callNativeMethod(OBJ_VAL(klass), AS_NATIVE_METHOD(method), argCount);
    }

    if (IS_BOUND_METHOD(method) || IS_CLOSURE(method))
    {
        return call(AS_CLOSURE(method), argCount);
    }

    if (IS_NIL(method))
    {
        runtimeError("'%s' has no method called '%s'.", string_get_cstr(klass->name), string_get_cstr(name));
        return false;
    }

    runtimeError("Can't call method '%s' from '%s'", string_get_cstr(name), string_get_cstr(klass->name));
    return false;
}

static bool callOperator(Value receiver, int argCount, OPERATOR operator)
{
    if (IS_INSTANCE(receiver) || IS_NATIVE_INSTANCE(receiver))
    {
        ObjInstance *instance = AS_INSTANCE(receiver);
        if (IS_NIL(instance->klass->operators[operator]))
        {
            runtimeError("Operator '%s' is not defined for class '%s'.",
                getOperatorString(operator), string_get_cstr(instance->klass->name));
            return false;
        }
        if (IS_NATIVE_METHOD(instance->klass->operators[operator]))
        {
            return callNativeMethod(receiver, AS_NATIVE_METHOD(instance->klass->operators[operator]), argCount);
        }
        else
        {
            return call(AS_CLOSURE(instance->klass->operators[operator]), argCount);
        }
    }
    runtimeError("Operators can only be called on object instances, got '%s'", objTypeName(AS_OBJ(receiver)->type));
    return false;
}

static bool invokeFromNativeInstance(ObjNativeInstance *instance, Value name, int argCount)
{
    Value method = findMethod(instance->instance.klass, name);
    if (IS_NIL(method))
    {
        runtimeError("'%s' has no method or property '%s'.",
            string_get_cstr(instance->instance.klass->name),
            string_get_cstr(name));
        return false;
    }

    return callNativeMethod(OBJ_VAL(instance), AS_NATIVE_METHOD(method), argCount);
}

static bool invoke(Value name, int argCount)
{
    Value receiver = peek(argCount);

    if (!(IS_INSTANCE(receiver) || IS_NATIVE_INSTANCE(receiver) || IS_CLASS(receiver) || IS_NATIVE_CLASS(receiver)))
    {
        if (IS_NIL(receiver))
        {
            runtimeError("'%s' can't be invoked from nil.", string_get_cstr(name));
        }
        else
        {
            runtimeError("'%s' can't be invoked from a '%s'.", string_get_cstr(name), objTypeName(OBJ_TYPE(receiver)));
        }
        return false;
    }

    if (IS_INSTANCE(receiver) || IS_NATIVE_INSTANCE(receiver))
    {
        ObjInstance *instance = AS_INSTANCE(receiver);

        // First look for a field which may shadow a method.
        Value value;
        if (tableGet(&instance->fields, name, &value))
        {
            // Load the field onto the stack in place of the receiver.
            vm.stackTop[-argCount - 1] = value;
            // Try to invoke it like a function.
            return callValue(value, argCount);
        }

        if (IS_NATIVE_INSTANCE(receiver))
        {
            return invokeFromNativeInstance(AS_NATIVE_INSTANCE(receiver), name, argCount);
        }

        return invokeFromClass(instance->klass, name, argCount);
    }
    return invokeFromClass(AS_CLASS(receiver), name, argCount);
}

Value nativeInvokeMethod(Value receiver, Value method_name, int arg_count, ...)
{
    push(receiver);
    va_list args;
    va_start(args, arg_count);
    for (int i = 0; i < arg_count; i++)
    {
        push(va_arg(args, Value));
    }
    va_end(args);

    if (invoke(method_name, arg_count))
    {
        return pop();
    }
    return NIL_VAL;
}

static bool bindMethod(ObjClass *klass, Value name)
{
    Value method;
    if (!tableGet(&klass->methods, name, &method))
    {
        runtimeError("Undefined method '%s'.", string_get_cstr(name));
        return false;
    }

    ObjBoundMethod *bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop(); // Instance.
    push(OBJ_VAL(bound));
    return true;
}

static ObjUpvalue *captureUpvalue(Value *local)
{
    ObjUpvalue *prevUpvalue = NULL;
    ObjUpvalue *upvalue = vm.openUpvalues;

    while (upvalue != NULL && upvalue->location > local)
    {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local)
        return upvalue;

    ObjUpvalue *createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;
    if (prevUpvalue == NULL)
    {
        vm.openUpvalues = createdUpvalue;
    }
    else
    {
        prevUpvalue->next = createdUpvalue;
    }
    return createdUpvalue;
}

static void closeUpvalues(Value *last)
{
    while (vm.openUpvalues != NULL &&
           vm.openUpvalues->location >= last)
    {
        ObjUpvalue *upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

void defineMethod(Value name, bool isStatic)
{
    Value method = peek(0);
    ObjClass *klass = AS_CLASS(peek(1));
    if (isStatic)
    {
        tableSet(&klass->staticMethods, name, method);
    }
    else
    {
        tableSet(&klass->methods, name, method);
    }
    pop();
}

void defineOperator(OPERATOR operator)
{
    Value method = peek(0);
    ObjClass *klass = AS_CLASS(peek(1));
    klass->operators[operator] = method;
    pop();
}

static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run(void)
{
    CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)                        \
    do                                                  \
    {                                                   \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
                                                        \
        double b = AS_NUMBER(pop());                    \
        double a = AS_NUMBER(pop());                    \
        push(valueType(a op b));                        \
    } while (false)

    for (;;)
    {
        uint8_t instruction;
#if DEBUG_TRACE_EXECUTION
        disassembleInstruction(&frame->closure->function->chunk,
                               (int)(frame->ip - frame->closure->function->chunk.code));
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
#endif
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_NIL:
            push(NIL_VAL);
            break;
        case OP_TRUE:
            push(TRUE_VAL);
            break;
        case OP_FALSE:
            push(FALSE_VAL);
            break;
        case OP_POP:
            pop();
            break;
        case OP_GET_GLOBAL:
        {
            Value name = READ_CONSTANT();
            Value value;
            if (!tableGet(&globals, name, &value))
            {
                runtimeError("Undefined variable '%s'.", string_get_cstr(name));
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            break;
        }
        case OP_DEFINE_GLOBAL:
        {
            Value name = READ_CONSTANT();
            tableSet(&globals, name, peek(0));
            pop();
            break;
        }
        case OP_GET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            push(frame->slots[slot]);
            break;
        }
        case OP_SET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(0);
            break;
        }
        case OP_SET_GLOBAL:
        {
            Value name = READ_CONSTANT();
            if (tableSet(&globals, name, peek(0)))
            {
                tableDelete(&globals, name);
                runtimeError("Undefined variable '%s'.", string_get_cstr(name));
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_GET_UPVALUE:
        {
            uint8_t slot = READ_BYTE();
            push(*frame->closure->upvalues[slot]->location);
            break;
        }
        case OP_SET_UPVALUE:
        {
            uint8_t slot = READ_BYTE();
            *frame->closure->upvalues[slot]->location = peek(0);
            break;
        }
        case OP_GET_PROPERTY:
        {
            if (!IS_INSTANCE(peek(0)))
            {
                runtimeError("Only instances have properties.");
                return INTERPRET_RUNTIME_ERROR;
            }
            ObjInstance *instance = AS_INSTANCE(peek(0));
            Value name = READ_CONSTANT();

            Value value;
            if (tableGet(&instance->fields, name, &value))
            {
                pop(); // Instance.
                push(value);
                break;
            }

            if (!bindMethod(instance->klass, name))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SET_PROPERTY:
        {
            if (!IS_INSTANCE(peek(1)))
            {
                runtimeError("Only instances have fields.");
                return INTERPRET_RUNTIME_ERROR;
            }
            ObjInstance *instance = AS_INSTANCE(peek(1));
            tableSet(&instance->fields, OBJ_VAL(READ_CONSTANT()), peek(0));

            Value value = pop();
            pop();
            push(value);
            break;
        }
        case OP_GET_SUPER:
        {
            Value name = READ_CONSTANT();
            ObjClass *superclass = AS_CLASS(pop());
            if (!bindMethod(superclass, name))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            break;
        }
        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;
        case OP_ADD:
        {
            if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
            {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            }
            else
            {
                int argCount = READ_BYTE();
                Value receiver = peek(argCount);
                if (!callOperator(receiver, argCount, OPERATOR_PLUS))
                {
                    return INTERPRET_RUNTIME_ERROR;;
                }
                frame = &vm.frames[vm.frameCount - 1];
            }
            break;
        }
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;
        case OP_NOT:
            push(BOOL_VAL(isFalsey(pop())));
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(peek(0)))
            {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        case OP_PRINT:
        {
            printValue(pop());
            printf("\n");
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_JUMP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            uint16_t offset = READ_SHORT();
            if (isFalsey(peek(0)))
                frame->ip += offset;
            break;
        }
        case OP_LOOP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            break;
        }
        case OP_CALL:
        {
            int argCount = READ_BYTE();
            if (!callValue(peek(argCount), argCount))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_INVOKE:
        {
            Value method = READ_CONSTANT();
            int argCount = READ_BYTE();
            if (!invoke(method, argCount))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_SUPER:
        {
            int argCount = READ_BYTE();
            Value method = READ_CONSTANT();
            ObjClass *superclass = AS_CLASS(pop());
            if (!invokeFromClass(superclass, method, argCount))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_CLOSURE:
        {
            ObjFunction *function = AS_FUNCTION(READ_CONSTANT());
            ObjClosure *closure = newClosure(function);
            push(OBJ_VAL(closure));
            for (int i = 0; i < closure->upvalueCount; i++)
            {
                uint8_t isLocal = READ_BYTE();
                uint8_t index = READ_BYTE();
                if (isLocal)
                {
                    closure->upvalues[i] = captureUpvalue(frame->slots + index);
                }
                else
                {
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }
            break;
        }
        case OP_CLOSE_UPVALUE:
            closeUpvalues(vm.stackTop - 1);
            pop();
            break;
        case OP_RETURN:
        {
            Value result = pop();
            closeUpvalues(frame->slots);
            vm.frameCount--;
            if (vm.frameCount == 0)
            {
                pop();
                return INTERPRET_OK;
            }

            vm.stackTop = frame->slots;
            push(result);

            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_CLASS:
            push(OBJ_VAL(newClass(READ_CONSTANT())));
            break;
        case OP_INHERIT:
        {
            Value super_ = peek(1);
            if (!(IS_CLASS(super_) || IS_NATIVE_CLASS(super_)))
            {
                runtimeError("Superclass must be a class.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjClass *subclass = AS_CLASS(peek(0));
            ObjClass *superclass = AS_CLASS(super_);
            tableAddAll(&superclass->methods, &subclass->methods);
            tableAddAll(&superclass->staticMethods, &subclass->staticMethods);
            for (int i = 0; i < NUM_OPERATORS; i++)
            {
                subclass->operators[i] = superclass->operators[i];
            }
            subclass->super_ = superclass;
            pop(); // Subclass.
            break;
        }
        case OP_METHOD:
            defineMethod(READ_CONSTANT(), false);
            break;
        case OP_STATIC_METHOD:
            defineMethod(READ_CONSTANT(), true);
            break;
        case OP_ENUM:
            // An enum is a class that inherits from Enum and has a bunch of static properties
            // That are instances of said class.
            // push(OBJ_VAL(newEnum(READ_STRING())));
            break;
        case OP_INDEX:
        {
            int argCount = READ_BYTE();
            Value receiver = peek(argCount);
            if (!callOperator(receiver, argCount, OPERATOR_INDEX))
            {
                return INTERPRET_RUNTIME_ERROR;;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_INDEX_ASSIGN:
        {
            int argCount = READ_BYTE();
            Value receiver = peek(argCount);
            if (!callOperator(receiver, argCount, OPERATOR_INDEX_ASSIGN))
            {
                return INTERPRET_RUNTIME_ERROR;;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_DEFINE_OPERATOR:
        {
            defineOperator((OPERATOR)READ_BYTE());
            break;
        }
        case OP_THROW:
        {
            ObjInstance *exception = AS_INSTANCE(peek(0));
            runtimeError("Uncaught %s", string_get_cstr(exception->klass->name));
            return INTERPRET_RUNTIME_ERROR;;
        }
        case OP_DUP_TOP:
        {
            push(peek(0));
            break;
        }
        }
    }

#undef BINARY_OP
#undef READ_STRING
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_BYTE
}

InterpretResult interpret(const SourceFile *source)
{
    ObjFunction *function = compile(source);
    if (function == NULL)
        return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure *closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    callValue(OBJ_VAL(closure), 0);

    return run();
}
