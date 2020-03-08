#include "comet.h"
#include "cometlib.h"

#include <stdlib.h>

typedef struct list_node {
    struct list_node *next;
    VALUE item;
} list_node_t;

typedef struct {
    list_node_t *head;
    list_node_t *tail;
    int length;
} ListData;


void *list_constructor(void)
{
    ListData *data = (ListData *) malloc(sizeof(ListData));
    data->length = 0;
    data->head = NULL;
    data->tail = NULL;
    return data;
}

void list_destructor(void *data)
{
    free(data);
}

VALUE list_add(VALUE self, int arg_count, VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    ListData *data = instance->data;
    for (int i = 0; i < arg_count; i++)
    {
        list_node_t *node = (list_node_t *) malloc(sizeof(list_node_t));
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
    return NIL_VAL;
}

VALUE list_get_at(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_iterable_empty_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_iterable_iterator(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_iterable_contains_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_sort(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_list(void)
{
    VALUE klass = defineNativeClass("List", list_constructor, list_destructor, NULL);
    defineNativeMethod(klass, list_add, "add", false);
}
