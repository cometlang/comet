#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vm.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"

#include "comet.h"

__thread VM vm;
static Table globals;
static Table strings;

static Value clockNative(int UNUSED(argCount), Value UNUSED(*args))
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

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

ObjString *findInternedString(const char *chars, const size_t length, uint32_t hash)
{
    return tableFindString(&strings, chars, length, hash);
}

bool internString(ObjString *string)
{
    return tableSet(&strings, string, NIL_VAL);
}

bool findGlobal(ObjString *name, Value *value)
{
    return tableGet(&globals, name, value);
}

bool addGlobal(ObjString *name, Value value)
{
    return tableSet(&globals, name, value);
}

static void resetStack(void)
{
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm.frames[i];
        ObjFunction *function = frame->closure->function;
        // -1 because the IP is sitting on the next instruction to be
        // executed.
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
                function->chunk.lines[instruction]);
        if (function->name == NULL)
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

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

    defineNative("clock", clockNative);
    init_stdlib();
}

void freeVM(void)
{
    freeTable(&globals);
    freeTable(&strings);
    vm.initString = NULL;
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
            vm.stackTop -= argCount + 1;
            push(result);
            return true;
        }

        default:
            printf("Obj type: %u\n", OBJ_TYPE(callee));
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
    vm.stackTop -= argCount + 1;
    push(result);
    return true;
}

static Value findMethod(ObjClass *klass, ObjString *name)
{
    Value method;
    if (!(tableGet(&klass->methods, name, &method)))
    {
        return NIL_VAL;
    }
    return method;
}

static bool invokeFromClass(ObjClass *klass, ObjString *name,
                            int argCount)
{
    Value method = findMethod(klass, name);
    if (IS_BOUND_METHOD(method) || IS_CLOSURE(method))
    {
        return call(AS_CLOSURE(method), argCount);
    }

    tableGet(&klass->staticMethods, name, &method);
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
        runtimeError("'%s' has no method called '%s'.", klass->name->chars, name->chars);
        return false;
    }

    runtimeError("Can't call method '%s' from '%s'", name->chars, klass->name->chars);
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
                getOperatorString(operator), instance->klass->name->chars);
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
    runtimeError("Operators can only be called on object instances, got '%s'", objTypeName(receiver));
    return false;
}

static bool invokeFromNativeInstance(ObjNativeInstance *instance, ObjString *name, int argCount)
{
    Value method = findMethod(instance->instance.klass, name);
    if (IS_NIL(method))
        return false;

    return callNativeMethod(OBJ_VAL(instance), AS_NATIVE_METHOD(method), argCount);
}

static bool invoke(ObjString *name, int argCount)
{
    Value receiver = peek(argCount);

    if (!(IS_INSTANCE(receiver) || IS_NATIVE_INSTANCE(receiver) || IS_CLASS(receiver) || IS_NATIVE_CLASS(receiver)))
    {
        runtimeError("'%s' can't be invoked from a '%s'.", name->chars, objTypeName(OBJ_TYPE(receiver)));
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

Value nativeInvokeMethod(Value receiver, ObjString *method_name, int arg_count, ...)
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
        // Should this be a peek or a pop?
        return peek(0);
    }
    return NIL_VAL;
}

static bool bindMethod(ObjClass *klass, ObjString *name)
{
    Value method;
    if (!tableGet(&klass->methods, name, &method))
    {
        runtimeError("Undefined method '%s'.", name->chars);
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

void defineMethod(ObjString *name, bool isStatic)
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

static void concatenate()
{
    ObjString *b = AS_STRING(peek(0));
    ObjString *a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run(void)
{
    CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
            push(BOOL_VAL(true));
            break;
        case OP_FALSE:
            push(BOOL_VAL(false));
            break;
        case OP_POP:
            pop();
            break;
        case OP_GET_GLOBAL:
        {
            ObjString *name = READ_STRING();
            Value value;
            if (!tableGet(&globals, name, &value))
            {
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            break;
        }
        case OP_DEFINE_GLOBAL:
        {
            ObjString *name = READ_STRING();
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
            ObjString *name = READ_STRING();
            if (tableSet(&globals, name, peek(0)))
            {
                tableDelete(&globals, name);
                runtimeError("Undefined variable '%s'.", name->chars);
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
            ObjString *name = READ_STRING();

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
            tableSet(&instance->fields, READ_STRING(), peek(0));

            Value value = pop();
            pop();
            push(value);
            break;
        }
        case OP_GET_SUPER:
        {
            ObjString *name = READ_STRING();
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
            if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
            {
                concatenate();
            }
            else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
            {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            }
            else
            {
                runtimeError("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
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
            ObjString *method = READ_STRING();
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
            ObjString *method = READ_STRING();
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
            push(OBJ_VAL(newClass(READ_STRING())));
            break;
        case OP_INHERIT:
        {
            Value superclass = peek(1);
            if (!(IS_CLASS(superclass) || IS_NATIVE_CLASS(superclass)))
            {
                runtimeError("Superclass must be a class.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjClass *subclass = AS_CLASS(peek(0));
            tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
            tableAddAll(&AS_CLASS(superclass)->staticMethods, &subclass->staticMethods);
            for (int i = 0; i < NUM_OPERATORS; i++)
            {
                subclass->operators[i] = AS_CLASS(superclass)->operators[i];
            }
            pop(); // Subclass.
            break;
        }
        case OP_METHOD:
            defineMethod(READ_STRING(), false);
            break;
        case OP_STATIC_METHOD:
            defineMethod(READ_STRING(), true);
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
        case OP_DEFINE_OPERATOR:
        {
            defineOperator((OPERATOR)READ_BYTE());
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

InterpretResult interpret(const char *source)
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
