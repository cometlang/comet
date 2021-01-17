#include <stdlib.h>
#include <string.h>

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

typedef struct StringData
{
    size_t length;
    char *chars;
    uint32_t hash;
} StringData;

uint32_t string_hash_cstr(const char *string, int length)
{
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++)
    {
        hash ^= string[i];
        hash *= 16777619;
    }

    return hash;
}

void *string_constructor(void)
{
    StringData *data = ALLOCATE(StringData, 1);
    data->length = 0;
    data->chars = NULL;
    data->hash = 0;
    return (void *) data;
}

void *string_set_cstr(ObjNativeInstance *instance, char *string, int length)
{
    StringData *data = (StringData *) instance->data;
    data->chars = string;
    data->length = length;
    data->hash = string_hash_cstr(data->chars, length);
    return (void *) data;
}

const char *string_get_cstr(VALUE self)
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    return data->chars;
}

int string_compare_to_cstr(VALUE self, const char *cstr)
{
    if (IS_INSTANCE(self) || IS_NATIVE_INSTANCE(self))
    {
        StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
        return strncmp(data->chars, cstr, data->length);
    }
    return -1;
}

void string_destructor(void *data)
{
    StringData *string_data = (StringData *) data;
    if (string_data->chars != NULL)
    {
        FREE_ARRAY(char, string_data->chars, string_data->length + 1);
        string_data->chars = NULL;
        string_data->length = 0;
        string_data->hash = 0;
    }
    FREE(StringData, string_data);
}

VALUE string_equals(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *lhs = GET_NATIVE_INSTANCE_DATA(StringData, self);
    StringData *rhs = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
    if (lhs->length != rhs->length)
        return FALSE_VAL;

    return BOOL_VAL(strncmp(lhs->chars, rhs->chars, lhs->length) == 0);
}

VALUE string_hash(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    return NUMBER_VAL(data->hash);
}

VALUE string_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return self;
}

VALUE string_trim(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_trim_left(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_trim_right(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_find(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_split(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_replace(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_starts_with_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_ends_with_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_to_lower(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_to_upper(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_empty_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = (StringData *) AS_NATIVE_INSTANCE(self)->data;
    return BOOL_VAL(data->length == 0);
}

VALUE string_concatenate(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_string(VM *vm, VALUE obj_klass)
{
    VALUE klass = bootstrapNativeClass(vm, "String", string_constructor, string_destructor);
    registerStringClass(klass);
    init_object(vm, obj_klass);
    completeNativeClassDefinition(vm, obj_klass, NULL);
    completeNativeClassDefinition(vm, klass, NULL);
    defineNativeMethod(vm, klass, &string_trim_left, "left_trim", false);
    defineNativeMethod(vm, klass, &string_trim_right, "right_trim", false);
    defineNativeMethod(vm, klass, &string_find, "find", false);
    defineNativeMethod(vm, klass, &string_split, "split", false);
    defineNativeMethod(vm, klass, &string_replace, "replace", false);
    defineNativeMethod(vm, klass, &string_starts_with_q, "starts_with?", false);
    defineNativeMethod(vm, klass, &string_ends_with_q, "ends_with?", false);
    defineNativeMethod(vm, klass, &string_empty_q, "empty?", false);
    defineNativeMethod(vm, klass, &string_to_lower, "to_lower", false);
    defineNativeMethod(vm, klass, &string_to_upper, "to_upper", false);
    defineNativeMethod(vm, klass, &string_to_string, "to_string", false);
    defineNativeOperator(vm, klass, &string_concatenate, OPERATOR_PLUS);
    defineNativeOperator(vm, klass, &string_equals, OPERATOR_EQUALS);
}