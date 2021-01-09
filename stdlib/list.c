#include "comet.h"
#include "cometlib.h"

#include <stdlib.h>

typedef struct list_node
{
    struct list_node *next;
    VALUE item;
} list_node_t;

typedef struct
{
    list_node_t *head;
    list_node_t *tail;
    int length;
} ListData;

void *list_constructor(void)
{
    ListData *data = (ListData *)malloc(sizeof(ListData));
    data->length = 0;
    data->head = NULL;
    data->tail = NULL;
    return data;
}

void list_destructor(void *data)
{
    FREE(ListData, data);
}

VALUE list_add(VALUE self, int arg_count, VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    ListData *data = instance->data;
    for (int i = 0; i < arg_count; i++)
    {
        list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));
        node->item = arguments[i];
        node->next = NULL;
        if (data->head == NULL)
        {
            data->head = data->tail = node;
        }
        else
        {
            data->tail->next = node;
            data->tail = node;
        }
        data->length++;
    }
    return NIL_VAL;
}

VALUE list_remove(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    // valuesEqual
    return NIL_VAL;
}

VALUE list_get_at(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    ListData *data = (ListData *)instance->data;
    int index = (int)AS_NUMBER(arguments[0]);
    list_node_t *current = data->head;
    for (int i = 0; i < data->length; i++)
    {
        if (i == index)
            return current->item;
        current = current->next;
    }
    return NIL_VAL;
}

VALUE list_assign_at(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance UNUSED(*instance) = AS_NATIVE_INSTANCE(self);
    int UNUSED(index) = (int)AS_NUMBER(arguments[0]);
    // If the index is larger than the current count, what do?
    // Probably throw an exception, except I can't do that yet...
    return NIL_VAL;
}

VALUE list_iterable_empty_q(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    ListData *data = (ListData *)instance->data;
    return BOOL_VAL(data->length == 0);
}

VALUE list_iterable_iterator(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_iterable_contains_q(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    ListData *data = (ListData *)instance->data;
    VALUE contains = arguments[0];
    list_node_t *current = data->head;
    while (current != NULL)
    {
        if (valuesEqual(current->item, contains))
            return TRUE_VAL;

        current = current->next;
    }
    return FALSE_VAL;
}

VALUE list_sort(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_obj_to_string(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    ListData *data = (ListData *)instance->data;
    VALUE contains = arguments[0];
    list_node_t *current = data->head;
    while (current != NULL)
    {
        if (valuesEqual(current->item, contains))
            return TRUE_VAL;

        current = current->next;
    }

    return NIL_VAL;
}

VALUE list_init(VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        uint64_t initial_length = AS_NUMBER(arguments[0]);
        VALUE args[initial_length];
        for (uint64_t i = 0; i < initial_length; i++)
        {
            args[i] = NIL_VAL;
        }
        list_add(self, initial_length, args);
    }
    return NIL_VAL;
}

void init_list(void)
{
    VALUE klass = defineNativeClass("List", list_constructor, list_destructor, "Iterable");
    defineNativeMethod(klass, &list_init, "init", false);
    defineNativeMethod(klass, &list_add, "add", false);
    defineNativeMethod(klass, &list_add, "push", false);
    defineNativeMethod(klass, &list_remove, "remove", false);
    defineNativeMethod(klass, &list_iterable_contains_q, "contains?", false);
    defineNativeMethod(klass, &list_iterable_empty_q, "empty?", false);
    defineNativeMethod(klass, &list_iterable_iterator, "iterator", false);
    defineNativeMethod(klass, &list_get_at, "get_at", false);
    defineNativeMethod(klass, &list_obj_to_string, "to_string", false);
    defineNativeOperator(klass, &list_get_at, OPERATOR_INDEX);
    defineNativeOperator(klass, &list_assign_at, OPERATOR_INDEX_ASSIGN);
}
