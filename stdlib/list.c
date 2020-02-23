#include "comet.h"
#include "cometlib.h"

#include <stdlib.h>

typedef struct {
    int length;
} ListData;


void *list_constructor(void)
{
    ListData *data = (ListData *) malloc(sizeof(ListData));
    data->length = 0;
    return data;
}

void list_destructor(void *data)
{
    free(data);
}

VALUE list_add(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
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
