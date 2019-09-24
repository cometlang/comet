#ifndef _ERAL_STACK_H_
#define _ERAL_STACK_H_
#include "list.h"

typedef struct stack eral_Stack_t;

eral_Stack_t *eral_StackCreate(void);

void eral_StackDelete(eral_Stack_t *stack, eral_NodeDestructor_fn destructorFn);

void eral_StackPush(eral_Stack_t *stack, uintptr_t data);

uintptr_t eral_StackPop(const eral_Stack_t *stack);

uintptr_t eral_StackPeek(const eral_Stack_t *stack);

bool_t eral_StackEmpty(const eral_Stack_t *stack);

uint32_t eral_StackNumItems(const eral_Stack_t *stack);

eral_Stack_t *eral_StackReverse(eral_Stack_t *stack);

#if ERAL_DEBUG
typedef void (*stackItemPrinter_t)(uintptr_t item);
void eral_DebugPrintStack(const eral_Stack_t *stack, stackItemPrinter_t);
#endif

#endif /* _ERAL_STACK_H_ */
