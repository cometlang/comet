#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

#include <stdlib.h>

typedef struct list_node
{
    VALUE item;
} list_node_t;

typedef struct
{
    list_node_t *entries;
    int count;
    int capacity;
} ListData;

void *list_constructor(void)
{
    ListData *data = (ListData *)malloc(sizeof(ListData));
    data->count = 0;
    data->capacity = 0;
    data->entries = NULL;
    return data;
}

void list_destructor(void *data)
{
    FREE(ListData, data);
}

VALUE list_add(VM UNUSED(*vm), VALUE self, int arg_count, VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    for (int i = 0; i < arg_count; i++)
    {
        if (data->capacity == data->count)
        {
            int new_capacity = GROW_CAPACITY(data->capacity);
            data->entries = GROW_ARRAY(data->entries, list_node_t, data->capacity, new_capacity);
            for (int i = data->capacity; i < new_capacity; i++)
            {
                data->entries[i].item = NIL_VAL;
            }
            data->capacity = new_capacity;
        }
        data->entries[data->count].item = arguments[i];
    }
    return NIL_VAL;
}

VALUE list_pop(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    data->count--;
    return data->entries[data->count].item;
}

VALUE list_get_at(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    if (arg_count == 1)
    {
        ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
        int index = (int)number_get_value(arguments[0]);
        return data->entries[index].item;
    }
    return NIL_VAL;
}

VALUE list_assign_at(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    int index = (int) number_get_value(arguments[0]);

    // If the index is larger than the current count, what do?
    // Probably throw an exception, except I can't do that yet...
    data->entries[index].item = arguments[1];
    return NIL_VAL;
}

VALUE list_iterable_empty_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    if(data->count == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE list_iterable_iterator(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_iterable_contains_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE contains = arguments[0];
    for (int i = 0; i < data->count; i++)
    {
        if (valuesEqual(data->entries[i].item, contains))
            return TRUE_VAL;
    }
    return FALSE_VAL;
}

VALUE list_sort(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE list_obj_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString(vm, "Native List Instance", 20);
}

VALUE list_length(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    return create_number(vm, (double) data->count);
}

VALUE list_init(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        int64_t initial_length = (int64_t) number_get_value(arguments[0]);
        VALUE args[initial_length];
        for (int64_t i = 0; i < initial_length; i++)
        {
            args[i] = NIL_VAL;
        }
        list_add(vm, self, initial_length, args);
    }
    return NIL_VAL;
}

void init_list(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "List", list_constructor, list_destructor, "Iterable");
    defineNativeMethod(vm, klass, &list_init, "init", false);
    defineNativeMethod(vm, klass, &list_add, "add", false);
    defineNativeMethod(vm, klass, &list_add, "push", false);
    defineNativeMethod(vm, klass, &list_pop, "pop", false);
    defineNativeMethod(vm, klass, &list_iterable_contains_q, "contains?", false);
    defineNativeMethod(vm, klass, &list_iterable_empty_q, "empty?", false);
    defineNativeMethod(vm, klass, &list_iterable_iterator, "iterator", false);
    defineNativeMethod(vm, klass, &list_get_at, "get_at", false);
    defineNativeMethod(vm, klass, &list_obj_to_string, "to_string", false);
    defineNativeMethod(vm, klass, &list_length, "size", false);
    defineNativeMethod(vm, klass, &list_length, "length", false);
    defineNativeOperator(vm, klass, &list_get_at, OPERATOR_INDEX);
    defineNativeOperator(vm, klass, &list_assign_at, OPERATOR_INDEX_ASSIGN);
}
