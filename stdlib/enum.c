#include <stdio.h>
#include <string.h>

#include "cometlib.h"
#include "comet_stdlib.h"

VALUE enum_class;
VALUE enum_iterator_class;
VALUE enum_value_class;

typedef struct {
    ObjNativeInstance obj;
    ValueArray array;
} EnumData;

typedef struct {
    ObjNativeInstance obj;
    EnumData *data;
    int index;
} EnumIterator;

typedef struct {
    ObjNativeInstance obj;
    double num;
    VALUE name;
} EnumValueData;

static void enumvalue_constructor(void *instanceData)
{
    EnumValueData *data = (EnumValueData *)instanceData;
    data->name = NIL_VAL;
    data->num = 0;
}

static VALUE enumvalue_init(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    EnumValueData *data = GET_NATIVE_INSTANCE_DATA(EnumValueData, self);
    data->name = arguments[0];
    data->num = number_get_value(arguments[1]);
    setNativeProperty(vm, self, "name", data->name);
    setNativeProperty(vm, self, "value", data->num);
    return NIL_VAL;
}

uint64_t enumvalue_get_value(VALUE instance)
{
    EnumValueData *data = GET_NATIVE_INSTANCE_DATA(EnumValueData, instance);
    return (uint64_t) data->num;
}

static VALUE enumvalue_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    EnumValueData *data = GET_NATIVE_INSTANCE_DATA(EnumValueData, self);
    const char *name = string_get_cstr(data->name);
#if WIN32
    #define max_len 256
#else
    int max_len = 64 + strlen(name);
#endif
    char temp_string[max_len];
    int length = snprintf(temp_string, max_len, "%s:%.17g", name, data->num);
    return copyString(vm, temp_string, length);
}


static void enum_iterator_constructor(void *instanceData)
{
    EnumIterator *data = (EnumIterator *)instanceData;
    data->data = NULL;
    data->index = 0;
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



static void enum_constructor(void *instanceData)
{
    EnumData *data = (EnumData *)instanceData;
    initValueArray(&data->array);
}

static void enum_destructor(void *to_destruct)
{
    EnumData *data = (EnumData *) to_destruct;
    freeValueArray(&data->array);
}

static VALUE enum_parse(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    VALUE candidate = arguments[0];
    if (IS_NUMBER(candidate))
    {
        for (int i = 0; i < data->array.count; i++)
        {
            VALUE current = data->array.values[i];
            if (current != NIL_VAL)
            {
                EnumValueData *val = GET_NATIVE_INSTANCE_DATA(EnumValueData, current);
                if (val->num == number_get_value(candidate))
                {
                    return current;
                }
            }
        }
    }
    else if (IS_INSTANCE_OF_STDLIB_TYPE(candidate, CLS_STRING))
    {
        for (int i = 0; i < data->array.count; i++)
        {
            VALUE current = data->array.values[i];
            if (current != NIL_VAL)
            {
                EnumValueData *val = GET_NATIVE_INSTANCE_DATA(EnumValueData, current);
                if (strcmp(string_get_cstr(val->name), string_get_cstr(candidate)) == 0)
                {
                    return current;
                }
            }
        }
    }
    return NIL_VAL;
}

static VALUE enum_add(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(enum_value_class)));
    push(vm, instance);
    enumvalue_init(vm, instance, arg_count, arguments);
    setNativeProperty(vm, self, string_get_cstr(arguments[0]), instance);
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    writeValueArray(&data->array, instance);
    pop(vm);
    return NIL_VAL;
}

static VALUE enum_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(enum_iterator_class)));
    enum_iterator_set_data(instance, GET_NATIVE_INSTANCE_DATA(EnumData, self));
    return instance;
}

static VALUE enum_contains_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    for (int i = 0; i < data->array.count; i++)
    {
        if (arguments[0] == data->array.values[i])
            return TRUE_VAL;
    }
    return FALSE_VAL;
}

static VALUE enum_empty_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
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

void enum_mark_contents(VALUE self)
{
    EnumData *data = GET_NATIVE_INSTANCE_DATA(EnumData, self);
    for (int i = 0; i < data->array.count; i++)
    {
        markValue(data->array.values[i]);
    }
}

VALUE enum_create(VM *vm)
{
    return OBJ_VAL(newInstance(vm, AS_CLASS(enum_class)));
}

void enum_add_value(VM *vm, VALUE enum_instance, const char *name, uint64_t value)
{
    VALUE args[2];
    args[0] = copyString(vm, name, strlen(name));
    push(vm, args[0]);
    args[1] = create_number(vm, value);
    push(vm, args[1]);
    enum_add(vm, enum_instance, 2, args);
    popMany(vm, 2);
}

void init_enum(VM *vm)
{
    enum_class = defineNativeClass(vm, "Enum", &enum_constructor, &enum_destructor, &enum_mark_contents, "Iterable", CLS_ENUM, sizeof(EnumData), true);
    defineNativeMethod(vm, enum_class, &enum_parse, "parse", 1, false);
    defineNativeMethod(vm, enum_class, &enum_add, "add", 2, false);
    defineNativeMethod(vm, enum_class, &enum_iterator, "iterator", 0, false);
    defineNativeMethod(vm, enum_class, &enum_contains_p, "contains?", 1, false);
    defineNativeMethod(vm, enum_class, &enum_empty_p, "empty?", 0, false);
    defineNativeMethod(vm, enum_class, &enum_length, "length", 0, false);
    defineNativeMethod(vm, enum_class, &enum_length, "count", 0, false);

    enum_value_class = defineNativeClass(vm, "EnumValue", &enumvalue_constructor, NULL, NULL, NULL, CLS_ENUM_VALUE, sizeof(EnumValueData), false);
    defineNativeMethod(vm, enum_value_class, &enumvalue_init, "init", 2, false);
    defineNativeMethod(vm, enum_value_class, &enumvalue_to_string, "to_string", 0, false);

    enum_iterator_class = defineNativeClass(
        vm, "EnumIterator", &enum_iterator_constructor, NULL, NULL, "Iterator", CLS_ITERATOR, sizeof(EnumIterator), false);
    defineNativeMethod(vm, enum_iterator_class, &enum_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, enum_iterator_class, &enum_iterator_get_next, "get_next", 0, false);
}
