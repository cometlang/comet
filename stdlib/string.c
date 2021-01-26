#include <stdlib.h>
#include <string.h>

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

#include "utf8proc.h"

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

    if (strncmp(lhs->chars, rhs->chars, lhs->length) == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE string_hash(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    return create_number(vm, (double) data->hash);
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

static utf8proc_int32_t to_lower_func(utf8proc_int32_t c, void UNUSED(*data))
{
    return utf8proc_tolower(c);
}

VALUE string_to_lower(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    char *dest_string;
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    utf8proc_ssize_t new_len = utf8proc_map_custom(
        (const utf8proc_uint8_t *)data->chars,
        data->length,
        (utf8proc_uint8_t **) &dest_string,
        UTF8PROC_NULLTERM,
        &to_lower_func,
        NULL);
    return takeString(vm, dest_string, new_len);
}

static utf8proc_int32_t to_upper_func(utf8proc_int32_t c, void UNUSED(*data))
{
    return utf8proc_toupper(c);
}

VALUE string_to_upper(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    char *dest_string;
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    utf8proc_ssize_t new_len = utf8proc_map_custom(
        (const utf8proc_uint8_t *)data->chars,
        data->length,
        (utf8proc_uint8_t **) &dest_string,
        UTF8PROC_NULLTERM,
        &to_upper_func,
        NULL);
    return takeString(vm, dest_string, new_len);
}

VALUE string_empty_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    if (data->length == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE string_length(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    utf8proc_int32_t current_codepoint;
    utf8proc_ssize_t remaining = data->length;
    size_t offset = 0;
    size_t length = 0;
    while (offset < data->length)
    {
        utf8proc_ssize_t bytes_read = utf8proc_iterate(
            (const utf8proc_uint8_t *) &data->chars[offset], remaining, &current_codepoint);
        if (bytes_read == -1)
            break;
        offset += bytes_read;
        remaining -= bytes_read;
        length++;
    }
    return create_number(vm, (double) length);
}

VALUE string_concatenate(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        StringData *lhs = GET_NATIVE_INSTANCE_DATA(StringData, self);
        StringData *rhs = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
        char *new_string = ALLOCATE(char, (lhs->length + rhs->length) + 1);
        memcpy(new_string, lhs->chars, lhs->length);
        memcpy(&new_string[lhs->length], rhs->chars, rhs->length);
        return takeString(vm, new_string, strlen(new_string));
    }
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
    defineNativeMethod(vm, klass, &string_length, "length", false);
    defineNativeOperator(vm, klass, &string_concatenate, OPERATOR_PLUS);
    defineNativeOperator(vm, klass, &string_equals, OPERATOR_EQUALS);
}