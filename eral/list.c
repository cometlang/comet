#include <stdlib.h>
#include <stdint.h>

#include "liberal.h"
#include "list.h"

struct list_node
{
   uintptr_t data;
   struct list_node *next;
   struct list_node *prev;
};

struct list
{
   eral_ListNode_t *head;
   eral_ListNode_t *tail;
   uint32_t nItems;
};

struct iterator
{
   eral_ListNode_t *current;
   eral_List_t *list;
};

static eral_ListNode_t *CreateListNode(uintptr_t data)
{
   eral_ListNode_t *node = (eral_ListNode_t *)malloc(sizeof(eral_ListNode_t));
   ASSERT(node != NULL);
   node->next = NULL;
   node->prev = NULL;
   node->data = data;
   return node;
}

eral_List_t *eral_ListCreate(void)
{
   eral_List_t *result = (eral_List_t *)malloc(sizeof(eral_List_t));
   ASSERT(result != NULL);
   result->head = NULL;
   result->tail = NULL;
   result->nItems = 0;
   return result;
}

void eral_ListDelete(eral_List_t *list, eral_NodeDestructor_fn destructorFn)
{
   eral_ListNode_t *current = NULL;
   eral_ListNode_t *next = NULL;

   current = list->head;

   while (current != NULL)
   {
      next = current->next;
      if (destructorFn != NULL)
      {
         destructorFn(current->data);
      }
      current->data = NULL_DATA;
      free(current);
      list->nItems--;
      current = next;
   }

   ASSERT(list->nItems == 0);
   list->head = NULL;
   list->tail = NULL;

   free(list);
}

void eral_ListAppendData(eral_List_t *list, uintptr_t data)
{
   eral_ListNode_t *node = CreateListNode(data);

   if (list->head == NULL)
   {
      list->head = node;
      list->tail = node;
   }
   else
   {
      list->tail->next = node;
      node->prev = list->tail;
      list->tail = node;
   }
   list->nItems++;
}

uintptr_t eral_ListRemoveTailData(eral_List_t *list)
{
   if (list->tail != NULL)
   {
      eral_ListNode_t *result_node = list->tail;
      uintptr_t result = list->tail->data;
      if (list->head == list->tail)
      {
         list->head = NULL;
         list->tail = NULL;
      }
      else
      {
         list->tail = list->tail->prev;
      }
      free(result_node);
      list->nItems--;
      return result;
   }
   return NULL_DATA;
}

uintptr_t eral_ListPeekTailData(eral_List_t *list)
{
   if (list->tail != NULL)
   {
      return list->tail->data;
   }
   return NULL_DATA;
}

eral_ListIterator_t *eral_ListGetIterator(eral_List_t *list)
{
   eral_ListIterator_t *iter = (eral_ListIterator_t *)malloc(sizeof(eral_ListIterator_t));
   ASSERT(iter != NULL);
   ASSERT(list != NULL);
   iter->list = list;
   iter->current = NULL;
   return iter;
}

eral_ListIterator_t *eral_ListCopyIterator(eral_ListIterator_t *iter)
{
   eral_ListIterator_t *result = (eral_ListIterator_t *)malloc(sizeof(eral_ListIterator_t));
   ASSERT(iter != NULL);
   ASSERT(result != NULL);
   result->list = iter->list;
   result->current = iter->current;
   return result;
}

void eral_ListDeleteIterator(eral_ListIterator_t *iter)
{
   ASSERT(iter != NULL);
   iter->current = NULL;
   iter->list = NULL;
   free(iter);
}

void eral_ListInsertDataAtCurrentPosition(eral_ListIterator_t *iter, uintptr_t data)
{
   eral_ListNode_t *node = CreateListNode(data);
   if (iter->current == NULL)
   {
      if (iter->list->head == NULL)
      {
         iter->list->head = node;
         iter->list->tail = node;
      }
      else
      {
         node->next = iter->list->head;
         node->next->prev = node;
         iter->list->head = node;
      }
   }
   else
   {
      node->next = iter->current->next;
      if (node->next != NULL)
      {
         node->next->prev = node;
      }
      node->prev = iter->current;
      iter->current->next = node;
      if (iter->current == iter->list->tail)
      {
         iter->list->tail = node;
      }
   }
   iter->current = node;
   iter->list->nItems++;
}

bool_t eral_ListEmpty(eral_List_t *list)
{
   return list->nItems == 0;
}

uint32_t eral_ListGetLength(eral_List_t *list)
{
   return list->nItems;
}

uint32_t eral_ListIteratorGetLength(eral_ListIterator_t *iter)
{
   return eral_ListGetLength(iter->list);
}

uintptr_t eral_ListPeekCurrentData(eral_ListIterator_t *iter)
{
   if (iter->current == NULL)
      return NULL_DATA;
   return iter->current->data;
}

uintptr_t eral_ListPeekNextData(eral_ListIterator_t *iter)
{
   if (iter->current == NULL || iter->current->next == NULL)
      return NULL_DATA;
   return iter->current->next->data;
}

uintptr_t eral_ListGetNextData(eral_ListIterator_t *iter)
{
   if (iter->current == iter->list->tail)
   {
      iter->current = NULL;
      return NULL_DATA;
   }

   if (iter->current == NULL)
   {
      iter->current = iter->list->head;
   }
   else
   {
      iter->current = iter->current->next;
   }
   return iter->current->data;
}

uintptr_t eral_ListGetPrevData(eral_ListIterator_t *iter)
{
   if (iter->current == iter->list->head)
   {
      iter->current = NULL;
      return NULL_DATA;
   }

   if (iter->current == NULL)
   {
      iter->current = iter->list->tail;
   }
   else
   {
      iter->current = iter->current->prev;
   }
   return iter->current->data;
}

void eral_ListConcatenate(eral_List_t *dst, eral_List_t *src)
{
   eral_ListIterator_t *iter = eral_ListGetIterator(src);
   uintptr_t current = eral_ListGetNextData(iter);
   while (current != NULL_DATA)
   {
      eral_ListAppendData(dst, current);
      current = eral_ListGetNextData(iter);
   }
   eral_ListDeleteIterator(iter);
   eral_ListDelete(src, NULL);
}

uintptr_t eral_ListRemoveCurrentData(eral_ListIterator_t *iter)
{
   eral_ListNode_t *current = iter->current;
   if (current == NULL)
      return NULL_DATA;

   if (current->prev != NULL)
   {
      current->prev->next = current->next;
   }
   else
   {
      iter->list->head = current->next;
   }

   if (current->next != NULL)
   {
      current->next->prev = current->prev;
   }
   else
   {
      iter->list->tail = current->prev;
   }
   iter->current = iter->current->prev;
   iter->list->nItems--;
   uintptr_t result = (uintptr_t)current->data;
   free(current);
   return (uintptr_t)result;
}
