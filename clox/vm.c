#include <stdio.h>
#include "vm.h"
#include "compiler.h"
#include "debug.h"

static VM vm;

static void resetStack(void)
{
    vm.stackTop = vm.stack;
}

void initVM(void)
{
    resetStack();
}

void freeVM(void) {}

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

static InterpretResult run(void)
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)   \
    do {                \
      double b = pop(); \
      double a = pop(); \
      push(a op b);     \
    } while (false)

    for (;;)
    {
        uint8_t instruction;
#if DEBUG_TRACE_EXECUTION
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
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
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            case OP_NEGATE:   push(-pop()); break;
            case OP_RETURN:
            {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
}

InterpretResult interpret(const char *source)
{
    compile(source);
    return run();
    // return INTERPRET_OK;
}
