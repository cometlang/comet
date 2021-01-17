#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object_defs.h"
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

struct _vm
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value *stackTop;
    ObjUpvalue *openUpvalues;
    Obj *objects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
};

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

typedef enum {
    STRING_INIT,
    STRING_HASH,
    STRING_TO_STRING,
    NUM_COMMON_STRINGS,
} COMMON_STRINGS;

extern Value common_strings[NUM_COMMON_STRINGS];
void initGlobals(void);
void freeGlobals(void);

void initVM(VM *vm);
void freeVM(VM *vm);

Value findInternedString(VM *vm, const char *chars, uint32_t hash);

bool internString(VM *vm, Value string);
void markGlobals(VM *vm);
void removeWhiteStrings(VM *vm);

bool addGlobal(VM *vm, Value name, Value value);
bool findGlobal(VM *vm, Value name, Value *value);

VALUE call_function(VALUE receiver, VALUE method_name, int arg_count, VALUE *arguments);

InterpretResult interpret(VM *vm, const SourceFile *source);
void runtimeError(VM *vm, const char *format, ...);
void defineMethod(VM *vm, Value name, bool isStatic);
void defineOperator(VM *vm, OPERATOR operator);

void push(VM *vm, Value value);
Value pop(VM *vm);
Value peek(VM *vm, int distance);

Value getStackTrace(VM *vm);

#endif
