#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "common.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

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
    ObjString *initString;
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

void initVM(void);

void freeVM(void);

ObjString *findInternedString(const char *chars, const size_t length, uint32_t hash);

bool internString(ObjString *string);
void markGlobals(void);
void removeWhiteStrings(void);

bool addGlobal(ObjString *name, Value value);
bool findGlobal(ObjString *name, Value *value);

InterpretResult interpret(const SourceFile *source);
void runtimeError(const char *format, ...);
void defineMethod(ObjString *name, bool isStatic);
void defineOperator(OPERATOR operator);

void push(Value value);
Value pop(void);
Value peek(int distance);

Value nativeInvokeMethod(Value receiver, ObjString *method_name, int arg_count, ...);

#endif
