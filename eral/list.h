#ifndef _ERAL_LIST_H_
#define _ERAL_LIST_H_

#include <stdint.h>
#include <limits.h>

/* Forward declarations of the opaque list types */
typedef struct list_node eral_ListNode_t;
typedef struct list eral_List_t;
typedef struct iterator eral_ListIterator_t;


typedef void (*eral_NodeDestructor_fn)(uintptr_t);

/**
 * Initialises an empty unordered, doubly-linked list.
 * Any call to this must be matched with a call to eral_ListDelete()
 */
eral_List_t *eral_ListCreate(void);

/**
 * Destroys the dynamic memory associated with both the list _AND_
 * all the items stored within it.
 * The supplied destructor function is for an individual data item in the list.
 */
void eral_ListDelete(eral_List_t *list, eral_NodeDestructor_fn destructorFn);

/**
 * Adds an item to the end of the list
 */
void eral_ListAppendData(eral_List_t *list, uintptr_t data);

/**
 * Removes the item at the tail (most recently appended) of the list.
 * The item removed is no longer part of the list and responsibility
 * for freeing any memory associated with the item is transferred to the client
 */
uintptr_t eral_ListRemoveTailData(eral_List_t *list);

uintptr_t eral_ListPeekTailData(eral_List_t *list);

/**
 * Returns True if the list has no items in it, otherwise false
 */
bool_t eral_ListEmpty(eral_List_t *list);

uint32_t eral_ListGetLength(eral_List_t *list);

/**
 * Gets an iterator pointing nowhere in the list.
 * The next call to eral_ListGetXXXXData determines whereabouts in the list we start.
 * If eral_ListGetNextData() is called first, we start at the beginning (head),
 * conversely if eral_ListGetPrevData() is called first, we start at the end (tail).
 * After this, any amount of Next or Prev may be called on any iterator, but if insertions
 * are made to the list (with another iterator) this may give unexpected results.
 * So long as there is nothing removed, the iterator is guaranteed to be valid,
 * but valid for the new version of the list.
 *
 * Any calls to this function should be matched with a call
 * to eral_ListDeleteIterator()
 */
eral_ListIterator_t *eral_ListGetIterator(eral_List_t *list);

/**
 * Destroys the dynamic memory associated with the iterator only.
 */
void eral_ListDeleteIterator(eral_ListIterator_t *iter);

/**
 * Copies an iterator for a list.  This means we can store whereabouts
 * in the list we are up to and continue to eral_ListInsertDataAtCurrentPosition()
 * without breaking the original iterator.
 * Any call to this function should be matched with a call to eral_ListDeleteIterator()
 */
eral_ListIterator_t *eral_ListCopyIterator(eral_ListIterator_t *iter);

/**
 * Inserts a new item in the list just after the position where the current iterator
 * is pointing and updates the iterator to point at the new item, so a call to
 * eral_ListGetNextData() will not return the new item, but the item which would
 * have been next in the list prior to a call to this function.
 */
void eral_ListInsertDataAtCurrentPosition(eral_ListIterator_t *iter, uintptr_t data);

/**
 * This shows what data the iterator is currently pointing at.
 * It has no side-effects on the iterator and callers *must not*
 * free the memory associated with the data returned.
 *
 * NULL is returned if the iterator isn't pointing at anything.
 */
uintptr_t eral_ListPeekCurrentData(eral_ListIterator_t *iter);

/**
 * This shows what data would be returned if the function eral_ListGetNextData
 * were called.  This does not alter the iterator position and callers
 * *must not* free the memory associated with the data returned.
 * 
 * NULL is returned if the iterator is not pointing at anything, or
 * the iterator is pointing at the tail of the list.
 */
uintptr_t eral_ListPeekNextData(eral_ListIterator_t *iter);

/**
 * This returns a copy of the head data. This differs from the
 * Peek Current Data, 
 */
uintptr_t eral_ListPeekTailData(eral_List_t *list);

/**
 * Returns the next item in the list or NULL if we are at the end of the list
 */
uintptr_t eral_ListGetNextData(eral_ListIterator_t *iter);

/**
 * Returns the previous item in the list or NULL if we are at the start of the list
 */
uintptr_t eral_ListGetPrevData(eral_ListIterator_t *iter);

/**
 * Concatenates the src list onto the dst list, freeing the memory
 * associated with the src list.
 */
void eral_ListConcatenate(eral_List_t *dst, eral_List_t *src);

/**
 * Removes the data at the current position and returns it to the caller.
 * If we don't have any data in the list, or the iterator has no current
 * data, it returns NULL.
 * This is a destructive operation.
 */
uintptr_t eral_ListRemoveCurrentData(eral_ListIterator_t *iter);

uint32_t eral_ListIteratorGetLength(eral_ListIterator_t *iter);

#endif /* _ERAL_LIST_H_ */
