#include <stdio.h>
#include <stdlib.h>

#define ERAL_DEBUG 1
#include "eral_config.h"
#include "liberal.h"
#include "stack.h"

#define NUM_TEST_DATA 10
static int basicTestData[NUM_TEST_DATA] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

static void stackItemPrinter(uintptr_t item)
{
   printf("Stack Item: %lu\n", item);
}

int main(void)
{
   int i;
   eral_Stack_t *stack = eral_StackCreate();

   for (i = 0; i < NUM_TEST_DATA; i++)
   {
      printf("Pushing %d\n", basicTestData[i]);
      eral_StackPush(stack, basicTestData[i]);
   }

   eral_DebugPrintStack(stack, stackItemPrinter);

   for (i = NUM_TEST_DATA - 1; i >= 0; i--)
   {
      int result = eral_StackPop(stack);
      printf("Popped %d\n", result);
      ASSERT(result == basicTestData[i]);
   }

   eral_StackDelete(stack, NULL);

   return EXIT_SUCCESS;
}
