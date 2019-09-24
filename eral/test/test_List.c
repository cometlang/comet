#include <stdio.h>

#define ERAL_DEBUG 1
#include "eral_config.h"
#include "liberal.h"
#include "list.h"

#define NUM_BASIC_TEST_ITEMS 10
#define NUM_ITEMS_TO_INSERT 6

int basic_test_data[NUM_BASIC_TEST_ITEMS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

int insertion_test_data[NUM_ITEMS_TO_INSERT] = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

static void test_BasicListFunctionality(void)
{
   int i;
   int result;
   eral_ListIterator_t *iter = NULL;
   eral_List_t *list = eral_ListCreate();

   printf("*** Setting up basic test data ***\n");

   for (i = 0; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      eral_ListAppendData(list, basic_test_data[i]);
   }

   printf("Getting Iterator\n");
   iter = eral_ListGetIterator(list);

   for (i = 0; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   result = (int)eral_ListGetNextData(iter);
   ASSERT(result == NULL_DATA);

   printf("Testing referential integrity going backwards, too!\n");
   for (i = NUM_BASIC_TEST_ITEMS - 1; i >= 0; i--)
   {
      result = (int)eral_ListGetPrevData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   printf("Cleaning up basic test\n");
   eral_ListDeleteIterator(iter);
   eral_ListDelete(list, NULL);
   printf("*** Finished basic test ***\n\n");
}

static void test_NullList(void)
{
   eral_ListIterator_t *iter = NULL;
   eral_List_t *list = eral_ListCreate();

   printf("*** Starting Null test ***\n");

   printf("Getting Iterator\n");
   iter = eral_ListGetIterator(list);

   ASSERT(eral_ListGetNextData(iter) == NULL_DATA);

   printf("Cleaning up Null test\n");
   eral_ListDeleteIterator(iter);
   eral_ListDelete(list, NULL);

   printf("*** Finished Null test ***\n\n");
}

static void test_InsertionWithIterator(void)
{
   int i;
   int result = NULL_DATA;
   eral_ListIterator_t *iter = NULL;
   eral_List_t *list = eral_ListCreate();

   printf("*** Starting insertion test ***\n");
   for (i = 0; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      eral_ListAppendData(list, basic_test_data[i]);
   }

   printf("Getting Iterator\n");
   iter = eral_ListGetIterator(list);

   for (i = 0; i < (NUM_BASIC_TEST_ITEMS / 2); i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   for (i = 0; i < NUM_ITEMS_TO_INSERT; i++)
   {
      eral_ListInsertDataAtCurrentPosition(iter, insertion_test_data[i]);
   }

   for (i = (NUM_BASIC_TEST_ITEMS / 2); i < NUM_BASIC_TEST_ITEMS; i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   result = (int)eral_ListGetNextData(iter);
   ASSERT(result == NULL_DATA);

   printf("Cleaning up insertion test\n");
   eral_ListDeleteIterator(iter);
   eral_ListDelete(list, NULL);

   printf("*** Finished insertion test ***\n\n");
}

static void test_CopyingIterator(void)
{
   int i;
   eral_ListIterator_t *iter = NULL;
   eral_ListIterator_t *iter_copy = NULL;
   eral_List_t *list = eral_ListCreate();
   int result = NULL_DATA;

   printf("*** Setting up basic iterator test data ***\n");

   for (i = 0; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      eral_ListAppendData(list, basic_test_data[i]);
   }

   printf("Getting initial Iterator\n");
   iter = eral_ListGetIterator(list);

   for (i = 0; i < NUM_BASIC_TEST_ITEMS / 2; i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   printf("Copying Iterator\n");
   iter_copy = eral_ListCopyIterator(iter);

   for (i = NUM_BASIC_TEST_ITEMS / 2; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }
   result = (int)eral_ListGetNextData(iter);
   ASSERT(result == NULL_DATA);

   for (i = NUM_BASIC_TEST_ITEMS / 2; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      result = (int)eral_ListGetNextData(iter_copy);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }
   result = (int)eral_ListGetNextData(iter_copy);
   ASSERT(result == NULL_DATA);

   printf("Cleaning up basic iterator test\n");
   eral_ListDeleteIterator(iter);
   eral_ListDeleteIterator(iter_copy);
   eral_ListDelete(list, NULL);
   printf("*** Finished basic iterator test ***\n\n");
}

static void test_InsertionIteratingBackwards(void)
{
   int i;
   eral_ListIterator_t *iter = NULL;
   eral_List_t *list = eral_ListCreate();
   int result;

   printf("*** Starting backwards insertion test ***\n");
   for (i = 0; i < NUM_BASIC_TEST_ITEMS; i++)
   {
      eral_ListAppendData(list, basic_test_data[i]);
   }

   printf("Getting Iterator\n");
   iter = eral_ListGetIterator(list);

   for (i = 0; i < (NUM_BASIC_TEST_ITEMS / 2); i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   for (i = 0; i < NUM_ITEMS_TO_INSERT; i++)
   {
      eral_ListInsertDataAtCurrentPosition(iter, insertion_test_data[i]);
   }

   for (i = (NUM_BASIC_TEST_ITEMS / 2); i < NUM_BASIC_TEST_ITEMS; i++)
   {
      result = (int)eral_ListGetNextData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   result = (int)eral_ListGetNextData(iter);
   ASSERT(result == NULL_DATA);

   printf("Turning around and testing backwards\n");
   for (i = NUM_BASIC_TEST_ITEMS - 1; i >= (NUM_BASIC_TEST_ITEMS / 2); i--)
   {
      result = (int)eral_ListGetPrevData(iter);
      ASSERT(result != NULL_DATA);
      printf("Expected: %d, Actual: %d\n", basic_test_data[i], result);
      ASSERT(basic_test_data[i] == result);
   }

   for (i = NUM_ITEMS_TO_INSERT - 1; i >= 0; i--)
   {
      result = (int)eral_ListGetPrevData(iter);
      ASSERT(result != NULL_DATA);
      printf("Expected: %d, Actual: %d\n", insertion_test_data[i], result);
      ASSERT(insertion_test_data[i] == result);
   }

   for (i = (NUM_BASIC_TEST_ITEMS / 2) - 1; i >= 0; i--)
   {
      result = (int)eral_ListGetPrevData(iter);
      ASSERT(result != NULL_DATA);
      ASSERT(basic_test_data[i] == result);
   }

   printf("Cleaning up backwards insertion test\n");
   eral_ListDeleteIterator(iter);
   eral_ListDelete(list, NULL);

   printf("*** Finished backwards insertion test ***\n\n");
}

static void test_Concatenate(void)
{
   printf("Testing concatenate...");
   eral_List_t *dst = eral_ListCreate();
   eral_List_t *src = eral_ListCreate();
   int i;
   for (i = 1; i <= 5; i++)
      eral_ListAppendData(dst, (uintptr_t)i);

   for (i = 6; i <= 10; i++)
      eral_ListAppendData(src, (uintptr_t)i);

   eral_ListConcatenate(dst, src);

   eral_ListIterator_t *iter = eral_ListGetIterator(dst);

   for (i = 1; i <= 10; i++)
   {
      int expected = (int)eral_ListGetNextData(iter);
      ASSERT(expected == i);
   }

   eral_ListDeleteIterator(iter);
   eral_ListDelete(dst, NULL);
   printf("ok\n");
}

static void test_RemoveCurrentData(void)
{
   printf("Testing remove current data...");
   eral_List_t *list = eral_ListCreate();
   int i;
   for (i = 0; i < NUM_BASIC_TEST_ITEMS; i++)
      eral_ListAppendData(list, (uintptr_t)basic_test_data[i]);

   eral_ListIterator_t *iter = eral_ListGetIterator(list);

   for (i = 0; i < NUM_BASIC_TEST_ITEMS / 2; i++)
      eral_ListGetNextData(iter);

   int result = (int)eral_ListRemoveCurrentData(iter);
   ASSERT(result == basic_test_data[(NUM_BASIC_TEST_ITEMS / 2) - 1]);
   int expected = (int)eral_ListGetNextData(iter);
   ASSERT(expected == basic_test_data[(NUM_BASIC_TEST_ITEMS / 2)]);

   eral_ListDeleteIterator(iter);
   eral_ListDelete(list, NULL);
   printf("ok\n");
}

int main(void)
{
   printf("Beginning %s\n", __FILE__);

   test_NullList();
   test_BasicListFunctionality();
   test_InsertionWithIterator();
   test_CopyingIterator();
   test_InsertionIteratingBackwards();
   test_Concatenate();
   test_RemoveCurrentData();

   printf("Finished %s\n", __FILE__);
   return 0;
}
