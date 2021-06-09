#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

#include <stdlib.h>
#include <stdio.h>

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

typedef struct {
    ListData *data;
    int index;
} ListIteratorData;

static VALUE list_class;
static VALUE list_iterator_class;

static void *list_iterator_constructor(void)
{
    ListIteratorData *data = ALLOCATE(ListIteratorData, 1);
    data->data = NULL;
    data->index = 0;
    return data;
}

static void list_iterator_destructor(void *data)
{
    FREE(ListIteratorData, data);
}

VALUE list_iterator_has_next_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListIteratorData *data = GET_NATIVE_INSTANCE_DATA(ListIteratorData, self);
    if (data->index < data->data->count)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE list_iterator_get_next(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListIteratorData *data = GET_NATIVE_INSTANCE_DATA(ListIteratorData, self);
    return data->data->entries[data->index++].item;
}

void *list_constructor(void)
{
    ListData *data = ALLOCATE(ListData, 1);
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
        data->entries[data->count++].item = arguments[i];
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

VALUE list_iterable_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(list_iterator_class)));
    ListIteratorData *list_iterator_data = GET_NATIVE_INSTANCE_DATA(ListIteratorData, instance);
    list_iterator_data->data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    return instance;
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

VALUE list_filter(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE result = list_create(vm);
    push(vm, result);
    for (int i = 0; i < data->capacity; i++)
    {
        VALUE status = call_function(NIL_VAL, arguments[0], 1, &data->entries[i].item);
        if (status == TRUE_VAL)
        {
            list_add(vm, result, 1, &data->entries[i].item);
        }
    }
    return pop(vm);
}

VALUE list_map(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE result = list_create(vm);
    push(vm, result);
    for (int i = 0; i < data->capacity; i++)
    {
        VALUE mapped_val = call_function(NIL_VAL, arguments[0], 1, &data->entries[i].item);
        list_add(vm, result, 1, &mapped_val);
    }
    return pop(vm);
}

VALUE list_reduce(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE accumulator = arguments[1];
    push(vm, accumulator);
    VALUE args[3];
    for (int i = 0; i < data->capacity; i++)
    {
        args[0] = accumulator;
        args[1] = data->entries[i].item;
        args[2] = create_number(vm, i);
        accumulator = call_function(NIL_VAL, arguments[0], 3, args);
    }
    return pop(vm);
}

VALUE list_find(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE callable = arguments[0];
    VALUE receiver = NIL_VAL;
    if (!callable_p(vm, 1, &callable))
    {
        if (IS_INSTANCE(callable) || IS_NATIVE_INSTANCE(callable))
        {
            receiver = callable;
            ObjInstance *instance = AS_INSTANCE(callable);
            callable = instance->klass->operators[OPERATOR_EQUALS];
        }
        else
        {
            throw_exception_native(
                vm,
                "ArgumentException",
                "Unable to compare '%s' in List.find",
                objTypeName(OBJ_TYPE(callable)));
            return NIL_VAL;
        }
    }

    for (int i = 0; i < data->capacity; i++)
    {
        VALUE result = call_function(receiver, callable, 1, &data->entries[i].item);
        if (result == TRUE_VAL)
            return data->entries[i].item;
    }
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
        int32_t initial_length = (int32_t) number_get_value(arguments[0]);
        VALUE *args = (VALUE *) malloc(sizeof(VALUE) * initial_length);
        for (int32_t i = 0; i < initial_length; i++)
        {
            args[i] = NIL_VAL;
        }
        list_add(vm, self, initial_length, args);
    }
    return NIL_VAL;
}

void list_mark_contents(VALUE self)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        if (!IS_NIL(data->entries[i].item))
        {
            markValue(data->entries[i].item);
        }
    }
}

VALUE list_create(VM *vm)
{
    return OBJ_VAL(newInstance(vm, AS_CLASS(list_class)));
}

void init_list(VM *vm)
{
    list_class = defineNativeClass(vm, "List", list_constructor, list_destructor, "Iterable", CLS_LIST);
    defineNativeMethod(vm, list_class, &list_init, "init", 1, false);
    defineNativeMethod(vm, list_class, &list_add, "add", 1, false);
    defineNativeMethod(vm, list_class, &list_add, "push", 1, false);
    defineNativeMethod(vm, list_class, &list_pop, "pop", 0, false);
    defineNativeMethod(vm, list_class, &list_iterable_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, list_class, &list_iterable_empty_q, "empty?", 0, false);
    defineNativeMethod(vm, list_class, &list_iterable_iterator, "iterator", 0, false);
    defineNativeMethod(vm, list_class, &list_get_at, "get_at", 1, false);
    defineNativeMethod(vm, list_class, &list_filter, "filter", 1, false);
    defineNativeMethod(vm, list_class, &list_map, "map", 1, false);
    defineNativeMethod(vm, list_class, &list_reduce, "reduce", 1, false);
    defineNativeMethod(vm, list_class, &list_find, "find", 1, false);
    defineNativeMethod(vm, list_class, &list_obj_to_string, "to_string", 0, false);
    defineNativeMethod(vm, list_class, &list_length, "size", 0, false);
    defineNativeMethod(vm, list_class, &list_length, "length", 0, false);
    defineNativeMethod(vm, list_class, &list_length, "count", 0, false);
    defineNativeOperator(vm, list_class, &list_get_at, 1, OPERATOR_INDEX);
    defineNativeOperator(vm, list_class, &list_assign_at, 2, OPERATOR_INDEX_ASSIGN);

    list_iterator_class = defineNativeClass(
        vm, "ListIterator", &list_iterator_constructor, &list_iterator_destructor, "Iterator", CLS_ITERATOR);
    defineNativeMethod(vm, list_iterator_class, &list_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, list_iterator_class, &list_iterator_get_next, "get_next", 0, false);
}
