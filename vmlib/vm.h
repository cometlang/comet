#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object_defs.h"
#include "table.h"
#include "value.h"
#include "common.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * MAX_VAR_COUNT)

typedef struct {
    uint16_t handlerAddress;
    uint16_t finallyAddress;
    Value klass;
} ExceptionHandler;

typedef struct
{
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
    uint8_t handlerCount;
    ExceptionHandler handlerStack[MAX_HANDLER_FRAMES];
    uint8_t bonusSplatArgCount;
} CallFrame;

struct _vm
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value *stackTop;
    ObjUpvalue *openUpvalues;
    int thread_id;
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
    STRING_LAMBDA,
    NUM_COMMON_STRINGS,
} COMMON_STRINGS;

extern Value common_strings[NUM_COMMON_STRINGS];
void initGlobals(void);
void freeGlobals(void);

void initVM(VM *vm);
void freeVM(VM *vm);

Value findInternedString(const char *chars, uint32_t hash);

bool internString(Value string);
void addModule(Value module, Value filename);
bool findModule(Value filename,  Value *module);
void markGlobals(void);
void removeWhiteStrings(void);

bool addGlobal(Value name, Value value);
bool findGlobal(Value name, Value *value);
bool findModuleVariable(Value module, Value name, Value *value);
bool addModuleVariable(Value module, Value name, Value value);

void call_function(VM *vm, VALUE receiver, VALUE method_name, int arg_count, VALUE *arguments);

InterpretResult interpret(VM *vm, Value main);
void runtimeError(VM *vm, const char *format, ...);
void defineMethod(VM *vm, Value name, bool isStatic);
void defineOperator(VM *vm, OPERATOR operator_);

void push(VM *vm, Value value);
void push_to(VM *vm, Value value, int distance);
Value pop(VM *vm);
Value peek(VM *vm, int distance);
Value popMany(VM *vm, int count);
void swapTop(VM *vm);

void throw_exception_native(VM *vm, const char *exception_type_name, const char *message_format, ...);

#if DEBUG_TRACE_EXECUTION
void toggle_stack_printing(void);
void print_stack(VM *vm);
void print_stack_trace(VM *vm);
#endif

#endif
