#include <stdlib.h>

#include "liberal.h"
#include "stack.h"
#include "list.h"

struct stack {
   eral_List_t *list;
};

eral_Stack_t *eral_StackCreate(void)
{
   eral_Stack_t *result = (eral_Stack_t *) malloc(sizeof(eral_Stack_t));
   result->list = eral_ListCreate();
   return result;
}

void eral_StackDelete(eral_Stack_t *stack, eral_NodeDestructor_fn destructorFn)
{
   eral_ListDelete(stack->list, destructorFn);
   free(stack);
}

void eral_StackPush(eral_Stack_t *stack, uintptr_t data)
{
   ASSERT(data != NULL_DATA);
   eral_ListAppendData(stack->list, data);
}

uintptr_t eral_StackPop(const eral_Stack_t *stack)
{
   return eral_ListRemoveTailData(stack->list);
}

uintptr_t eral_StackPeek(const eral_Stack_t *stack)
{
   return eral_ListPeekTailData(stack->list);
}

bool_t eral_StackEmpty(const eral_Stack_t *stack)
{
   return eral_ListEmpty(stack->list);
}

uint32_t eral_StackNumItems(const eral_Stack_t *stack)
{
   return eral_ListGetLength(stack->list);
}

eral_Stack_t *eral_StackReverse(eral_Stack_t *stack)
{
   eral_Stack_t *result = eral_StackCreate();
   while(!eral_StackEmpty(stack))
   {
      eral_StackPush(result, eral_StackPop(stack));
   }
   eral_StackDelete(stack, NULL);
   return result;
}

#if ERAL_DEBUG
void eral_DebugPrintStack(const eral_Stack_t *stack, stackItemPrinter_t itemPrinterFn)
{
   eral_ListIterator_t *iter = eral_ListGetIterator(stack->list);
   uintptr_t item = eral_ListGetPrevData(iter);
   printf("Number of Stack Items: %d\n", eral_ListGetLength(stack->list));
   while(item != NULL_DATA)
   {
      itemPrinterFn(item);
      item = eral_ListGetPrevData(iter);
   }
   eral_ListDeleteIterator(iter);
}
#endif
