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

static uint32_t hashString(const char *key, int length)
{
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++)
    {
        hash ^= key[i];
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

void *string_set_cstr(ObjNativeInstance *instance, const char *string, int length)
{
    StringData *data = (StringData *) instance->data;
    data->chars = ALLOCATE(char, length + 1);
    memcpy(data->chars, string, length);
    data->chars[length] = '\0';
    data->length = length;
    data->hash = hashString(data->chars, length);
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

VALUE string_equals(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *lhs = (StringData *) AS_NATIVE_INSTANCE(self)->data;
    StringData *rhs = (StringData *) AS_NATIVE_INSTANCE(arguments[0])->data;
    if (lhs->length != rhs->length)
        return FALSE_VAL;

    return BOOL_VAL(strncmp(lhs->chars, rhs->chars, lhs->length) == 0);
}

VALUE string_hash(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    return NUMBER_VAL(data->hash);
}

VALUE string_to_string(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return self;
}

VALUE string_trim(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_trim_left(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_trim_right(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_find(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_split(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_replace(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_starts_with_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_ends_with_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_to_lower(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_to_upper(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_empty_q(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = (StringData *) AS_NATIVE_INSTANCE(self)->data;
    return BOOL_VAL(data->length == 0);
}

VALUE string_concatenate(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_string(VALUE obj_klass)
{
    VALUE klass = bootstrapNativeClass("String", string_constructor, string_destructor);
    registerStringClass(klass);
    completeNativeClassDefinition(obj_klass, NULL);
    completeNativeClassDefinition(klass, NULL);
    defineNativeMethod(klass, &string_hash, "hash", false);
    defineNativeMethod(klass, &string_to_string, "to_string", false);
    defineNativeMethod(klass, &string_trim, "trim", false);
    defineNativeMethod(klass, &string_trim_left, "left_trim", false);
    defineNativeMethod(klass, &string_trim_right, "right_trim", false);
    defineNativeMethod(klass, &string_find, "find", false);
    defineNativeMethod(klass, &string_split, "split", false);
    defineNativeMethod(klass, &string_replace, "replace", false);
    defineNativeMethod(klass, &string_starts_with_q, "starts_with?", false);
    defineNativeMethod(klass, &string_ends_with_q, "ends_with?", false);
    defineNativeMethod(klass, &string_empty_q, "empty?", false);
    defineNativeMethod(klass, &string_to_lower, "to_lower", false);
    defineNativeMethod(klass, &string_to_upper, "to_upper", false);
    defineNativeOperator(klass, &string_concatenate, OPERATOR_PLUS);
    defineNativeOperator(klass, &string_equals, OPERATOR_EQUALS);
}