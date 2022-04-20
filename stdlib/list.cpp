#include <stdlib.h>
#include <stdio.h>

#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"


typedef struct list_node
{
    VALUE item;
} list_node_t;

typedef struct
{
    ObjNativeInstance obj;
    list_node_t *entries;
    int count;
    int capacity;
} ListData;

typedef struct {
    ObjNativeInstance obj;
    ListData *data;
    int index;
} ListIteratorData;

static VALUE list_class;
static VALUE list_iterator_class;

static void list_iterator_constructor(void *instanceData)
{
    ListIteratorData *data = (ListIteratorData *)instanceData;
    data->data = NULL;
    data->index = 0;
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

void list_constructor(void *instanceData)
{
    ListData *data = (ListData *)instanceData;
    data->count = 0;
    data->capacity = 0;
    data->entries = NULL;
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

VALUE list_union(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    ListData *other = GET_NATIVE_INSTANCE_DATA(ListData, arguments[0]);
    VALUE result = list_create(vm);
    push(vm, result);
    ListData *result_data = GET_NATIVE_INSTANCE_DATA(ListData, result);
    result_data->capacity = data->count + other->count;
    result_data->entries = ALLOCATE(list_node_t, result_data->capacity);
    for (int i = 0; i < data->count; i++)
    {
        result_data->entries[result_data->count++].item = data->entries[i].item;
    }
    for (int i = 0; i < other->count; i++)
    {
        result_data->entries[result_data->count++].item = other->entries[i].item;
    }
    return pop(vm);
}

VALUE list_pop(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    data->count--;
    return data->entries[data->count].item;
}

VALUE list_peek(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData* data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    return data->entries[data->count - 1].item;
}

VALUE list_get_at(VM UNUSED(*vm), VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
        VALUE arg = arguments[0];
        int index;
        if (IS_NUMBER(arg))
        {
            index = (int)number_get_value(arg);
        }
        else if (IS_INSTANCE_OF_STDLIB_TYPE(arg, CLS_ENUM_VALUE))
        {
            index = (int)enumvalue_get_value(arg);
        }
        if (index >= data->count)
        {
            throw_exception_native(
                vm,
                "IndexOutOfBoundsException",
                "Index (%d) was outside the bounds of the list", index);
            return NIL_VAL;
        }
        if (index < 0) {
            index = data->count + index;
        }
        return data->entries[index].item;
    }
    return NIL_VAL;
}

VALUE list_assign_at(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    int index = (int) number_get_value(arguments[0]);

    if (index >= data->count) {
        throw_exception_native(
                vm,
                "IndexOutOfBoundsException",
                "Index (%d) was outside the bounds of the list", index);
        return NIL_VAL;
    }
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
    VALUE compare_func = AS_INSTANCE(contains)->klass->operators[OPERATOR_EQUALS];
    for (int i = 0; i < data->count; i++)
    {
        VALUE result = call_function(contains, compare_func, 1, &data->entries[i].item);
        if (result == TRUE_VAL) {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
}

static VALUE is_lhs_less_than_or_equal_to_rhs(VALUE lhs, VALUE rhs, VALUE compare_func)
{
    if (IS_NUMBER(lhs))
    {
        if (number_get_value(lhs) <= number_get_value(rhs))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    return call_function(lhs, compare_func, 1, &rhs);
}

static VALUE get_compare_func(VM *vm, VALUE self)
{
    if (IS_INSTANCE(self) || IS_NATIVE_INSTANCE(self))
    {
        VALUE compare_func = AS_INSTANCE(self)->klass->operators[OPERATOR_LESS_EQUAL];
        if (compare_func == NIL_VAL)
        {
            runtimeError(
                vm,
                "ArgumentException",
                "%s doesn't implement <= as required for sorting",
                getClassNameFromInstance(self));
        }
    }
    return NIL_VAL;
}

static void insertion_sort(VM *vm, const ListData *data, int left, int right)
{
    for (int i = left + 1; i <= right; i++)
    {
        VALUE temp = data->entries[i].item;
        VALUE compare_func = get_compare_func(vm, temp);
        if (!IS_INSTANCE(temp) && !IS_NATIVE_INSTANCE(temp) && !IS_NUMBER(temp))
        {
            throw_exception_native(vm, "ArgumentException", "Can't sort a list containing a '%s'", objTypeName(AS_OBJ(temp)->type));
            return;
        }
        int j = i - 1;
        while (j >= left)
        {
            VALUE result = is_lhs_less_than_or_equal_to_rhs(temp, data->entries[j].item, compare_func);
            if (result == TRUE_VAL)
            {
                data->entries[j + 1].item = data->entries[j].item;
            }
            else
            {
                break;
            }
            j--;
        }
        data->entries[j + 1].item = temp;
    }
}

static void merge_sorted_runs(VM *vm, ListData* data, int l, int m, int r)
{
    // Original array is broken into two parts - left and right array
    int len1 = m - l + 1, len2 = r - m;
    VALUE* left = ALLOCATE(VALUE, len1);
    VALUE* right = ALLOCATE(VALUE, len2);
    for (int i = 0; i < len1; i++)
        left[i] = data->entries[l + i].item;
    for (int i = 0; i < len2; i++)
        right[i] = data->entries[m + 1 + i].item;

    int i = 0;
    int j = 0;
    int k = l;

    // After comparing, we merge those two arrays into a larger sub array
    while (i < len1 && j < len2)
    {
        VALUE compare_func = get_compare_func(vm, left[i]);
        VALUE result = is_lhs_less_than_or_equal_to_rhs(left[i], right[j], compare_func);
        if (result == TRUE_VAL)
        {
            data->entries[k].item = left[i];
            i++;
        }
        else
        {
            data->entries[k].item = right[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements of left, if any
    while (i < len1)
    {
        data->entries[k].item = left[i];
        k++;
        i++;
    }

    // Copy remaining element of right, if any
    while (j < len2)
    {
        data->entries[k].item = right[j];
        k++;
        j++;
    }

    FREE_ARRAY(VALUE, left, len1);
    FREE_ARRAY(VALUE, right, len2);
}

static constexpr int RUN = 32;

VALUE list_sort(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    for (int i = 0; i < data->count; i += RUN)
        insertion_sort(vm, data, i, min((i + RUN - 1), (data->count - 1)));

    // Start merging from size RUN (or 32).
    // It will merge to form size 64, then 128, 256 and so on...
    for (int size = RUN; size < data->count; size = 2 * size)
    {
        // pick starting point of left sub array. We are going to merge
        // arr[left..left+size-1] and arr[left+size, left+2*size-1]
        // After every merge, we increase left by 2*size
        for (int left = 0; left < data->count; left += 2 * size)
        {
            // find ending point of left sub array mid+1 is
            // the starting point of right sub array
            int mid = left + size - 1;
            int right = min((left + 2 * size - 1), (data->count - 1));

            // merge sub array arr[left.....mid] and arr[mid+1....right]
            if (mid < right)
                merge_sorted_runs(vm, data, left, mid, right);
        }
    }

    return self;
}

VALUE list_filter(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE result = list_create(vm);
    push(vm, result);
    for (int i = 0; i < data->count; i++)
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
    for (int i = 0; i < data->count; i++)
    {
        VALUE mapped_val = call_function(NIL_VAL, arguments[0], 1, &data->entries[i].item);
        list_add(vm, result, 1, &mapped_val);
    }
    return pop(vm);
}

VALUE list_reduce(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    VALUE accumulator = arguments[0];
    VALUE args[3];
    push(vm, accumulator);
    for (int i = 0; i < data->count; i++)
    {
        args[0] = accumulator;
        args[1] = data->entries[i].item;
        args[2] = create_number(vm, i);
        push(vm, args[2]);
        accumulator = call_function(NIL_VAL, arguments[1], 3, args);
        popMany(vm, 2);
        push(vm, accumulator);
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

VALUE list_obj_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ListData *data = GET_NATIVE_INSTANCE_DATA(ListData, self);
    std::stringstream stream;
    stream << "[";
    for (int i = 0; i < data->count; i++)
    {
        VALUE str = call_function(data->entries[i].item, common_strings[STRING_TO_STRING], 0, NULL);
        stream << string_get_cstr(str);
        if (i != data->count - 1)
            stream << ", ";
    }
    stream << "]";
    std::string result = stream.str();
    return copyString(vm, result.c_str(), result.length());
}

VALUE list_length(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
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
    list_class = defineNativeClass(vm, "List", list_constructor, NULL, "Iterable", CLS_LIST, sizeof(ListData), true);
    defineNativeMethod(vm, list_class, &list_init, "init", 1, false);
    defineNativeMethod(vm, list_class, &list_add, "add", 1, false);
    defineNativeMethod(vm, list_class, &list_add, "append", 1, false);
    defineNativeMethod(vm, list_class, &list_add, "push", 1, false);
    defineNativeMethod(vm, list_class, &list_pop, "pop", 0, false);
    defineNativeMethod(vm, list_class, &list_peek, "peek", 0, false);
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
    defineNativeMethod(vm, list_class, &list_sort, "sort", 0, false);
    defineNativeOperator(vm, list_class, &list_get_at, 1, OPERATOR_INDEX);
    defineNativeOperator(vm, list_class, &list_assign_at, 2, OPERATOR_INDEX_ASSIGN);
    defineNativeOperator(vm, list_class, &list_union, 1, OPERATOR_PLUS);

    list_iterator_class = defineNativeClass(
        vm, "ListIterator", &list_iterator_constructor, NULL, "Iterator", CLS_ITERATOR, sizeof(ListIteratorData), false);
    defineNativeMethod(vm, list_iterator_class, &list_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, list_iterator_class, &list_iterator_get_next, "get_next", 0, false);
}

#ifdef __cplusplus
}
#endif
