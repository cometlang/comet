#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "objects.h"
#include "table.h"
#include "value.h"
#include "common.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * MAX_VAR_COUNT)

#define str(x) #x
#define enum_str(x) str(x)

typedef struct
{
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef struct
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value *stackTop;
    Value initString;
    ObjUpvalue *openUpvalues;
    size_t bytesAllocated;
    size_t nextGC;
    Obj *objects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern __thread VM vm;

void initVM(VM *vm);

void freeVM(VM *vm);

Value findInternedString(const char *chars, uint32_t hash);

bool internString(Value string);
void markGlobals(void);
void removeWhiteStrings(void);

bool addGlobal(Value name, Value value);
bool findGlobal(Value name, Value *value);

InterpretResult interpret(VM *vm, const SourceFile *source);
void runtimeError(const char *format, ...);
void defineMethod(Value name, bool isStatic);
void defineOperator(OPERATOR operator);

void push(VM *vm, Value value);
Value pop(VM *vm);
Value peek(int distance);

Value nativeInvokeMethod(Value receiver, Value method_name, int arg_count, ...);

#endif
