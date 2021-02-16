#include <stdio.h>
#include <string.h>

#include "cometlib.h"
#include "comet_stdlib.h"

VALUE enum_class;
VALUE enum_iterator_class;
VALUE enum_value_class;

typedef struct {
    ValueArray array;
} EnumData;

typedef struct {
    EnumData *data;
    int index;
} EnumIterator;

typedef struct {
    NumberData num;
    VALUE name;
} EnumValueData;

static void *enumvalue_constructor(void)
{
    EnumValueData *data = ALLOCATE(EnumValueData, 1);
    data->name = NIL_VAL;
    data->num.num = 0;
    return data;
}

static void enumvalue_destructor(void *data)
{
    FREE(EnumValueData, data);
}

static VALUE enumvalue_init(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    EnumValueData *data = GET_NATIVE_INSTANCE_DATA(EnumValueData, self);
    data->name = arguments[0];
    data->num.num = number_get_value(arguments[1]);
    setNativeProperty(vm, self, "name", data->name);
    setNativeProperty(vm, self, "value", data->num.num);
    return NIL_VAL;
}

static VALUE enumvalue_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    EnumValueData *data = GET_NATIVE_INSTANCE_DATA(EnumValueData, self);
    const char *name = string_get_cstr(data->name);
    int max_len = 64 + strlen(name);
    char temp_string[max_len];
    int length = snprintf(temp_string, max_len, "%s:%.17g", name, data->num.num);
    return copyString(vm, temp_string, length);
}


static void *enum_iterator_constructor(void)
{
    EnumIterator *data = ALLOCATE(EnumIterator, 1);
    data->data = NULL;
    data->index = 0;
    return data;
}

static void enum_iterator_destructor(void *data)
{
    FREE(EnumIterator, data);
}

static void enum_iterator_set_data(VALUE self, EnumData *enum_data)
{
    EnumIterator *iter = GET_NATIVE_INSTANCE_DATA(EnumIterator, self);
    iter->data = enum_data;
}

static VALUE enum_iterator_has_next_p(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    EnumIterator *iter = GET_NATIVE_INSTANCE_DATA(EnumIterator, self);
    if (iter->index < iter->data->array.count)
        return TRUE_VAL;
    return FALSE_VAL;
}

static VALUE enum_iterator_get_next(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    EnumIterator *iter = GET_NATIVE_INSTANCE_DATA(EnumIterator, self);
    return iter->data->array.values[iter->index++];
}



static void *enum_constructor(void)
{
    EnumData *data = ALLOCATE(EnumData, 1);
    initValueArray(&data->array);
    return data;
}

static void enum_destructor(void *to_destruct)
{
    EnumData *data = (EnumData *) to_destruct;
    freeValueArray(&data->array);
    FREE(EnumData, data);
}

static VALUE enum_parse(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    // handle string names, string ints and int ints.
    return NIL_VAL;
}

static VALUE enum_add(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(enum_value_class)));
    enumvalue_init(vm, instance, arg_count, arguments);
    setNativeProperty(vm, self, string_get_cstr(arguments[0]), instance);
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    writeValueArray(&data->array, instance);
    return NIL_VAL;
}

static VALUE enum_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(enum_iterator_class)));
    enum_iterator_set_data(instance, GET_NATIVE_INSTANCE_DATA(EnumData, self));
    return instance;
}

static VALUE enum_contains_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    for (int i = 0; i < data->array.count; i++)
    {
        if (arguments[0] == data->array.values[i])
            return TRUE_VAL;
    }
    return FALSE_VAL;
}

static VALUE enum_empty_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    if (data->array.count == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

static VALUE enum_length(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    return create_number(vm, data->array.count);
}

void init_enum(VM *vm)
{
    enum_class = defineNativeClass(vm, "Enum", &enum_constructor, &enum_destructor, "Iterable", CLS_ENUM);
    defineNativeMethod(vm, enum_class, &enum_parse, "parse", 1, true);
    defineNativeMethod(vm, enum_class, &enum_add, "add", 2, false);
    defineNativeMethod(vm, enum_class, &enum_iterator, "iterator", 0, false);
    defineNativeMethod(vm, enum_class, &enum_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, enum_class, &enum_empty_q, "empty?", 0, false);
    defineNativeMethod(vm, enum_class, &enum_length, "length", 0, false);

    enum_value_class = defineNativeClass(vm, "EnumValue", &enumvalue_constructor, &enumvalue_destructor, "Number", CLS_ENUM_VALUE);
    defineNativeMethod(vm, enum_value_class, &enumvalue_init, "init", 2, false);
    defineNativeMethod(vm, enum_value_class, &enumvalue_to_string, "to_string", 0, false);

    enum_iterator_class = defineNativeClass(
        vm, "EnumIterator", &enum_iterator_constructor, &enum_iterator_destructor, "Iterator", CLS_ITERATOR);
    defineNativeMethod(vm, enum_iterator_class, &enum_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, enum_iterator_class, &enum_iterator_get_next, "get_next", 0, false);
}
