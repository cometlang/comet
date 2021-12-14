#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

#include "utf8proc.h"

static VALUE string_iterator_class;

typedef struct
{
    ObjNativeInstance obj;
    size_t length;
    char *chars;
    uint32_t hash;
} StringData;

typedef struct
{
    ObjNativeInstance obj;
    StringData *string;
    utf8proc_int32_t current_codepoint;
    utf8proc_ssize_t remaining;
    utf8proc_ssize_t offset;
} StringIterator;

static VALUE string_class;

void string_iterator_constructor(void *data)
{
    StringIterator *iter = (StringIterator *)data;
    iter->string = NULL;
    iter->current_codepoint = 0;
    iter->remaining = 0;
    iter->offset = 0;
}

static VALUE string_iterator_has_next_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringIterator *data = GET_NATIVE_INSTANCE_DATA(StringIterator, self);
    if (data->remaining > 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

static VALUE string_iterator_get_next(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringIterator *data = GET_NATIVE_INSTANCE_DATA(StringIterator, self);
    utf8proc_ssize_t bytes_read = utf8proc_iterate(
        (const utf8proc_uint8_t *)&data->string->chars[data->offset], data->remaining, &data->current_codepoint);
    if (bytes_read == -1)
        return NIL_VAL;
    data->offset += bytes_read;
    data->remaining -= bytes_read;

    utf8proc_uint8_t character[4];
    utf8proc_ssize_t char_len = utf8proc_encode_char(data->current_codepoint, character);
    return copyString(vm, (const char *) character, (int)char_len);
}


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

void string_constructor(void *instanceData)
{
    StringData *data = (StringData *)instanceData;
    data->length = 0;
    data->chars = NULL;
    data->hash = 0;
}

VALUE string_create(VM *vm, char *chars, int length)
{
    VALUE string = OBJ_VAL(newInstance(vm, AS_CLASS(string_class)));
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, string);
    data->chars = chars;
    data->length = length;
    data->hash = string_hash_cstr(data->chars, length);

    return string;
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
    StringData *string_data = (StringData *)data;
    if (string_data->chars != NULL)
    {
        FREE_ARRAY(char, string_data->chars, string_data->length + 1);
        string_data->chars = NULL;
        string_data->length = 0;
        string_data->hash = 0;
    }
}

VALUE string_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(string_iterator_class)));
    StringIterator *iter = GET_NATIVE_INSTANCE_DATA(StringIterator, instance);
    iter->string = data;
    iter->remaining = data->length;
    return instance;
}

VALUE string_equals(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    VALUE rhs = arguments[0];
    if (IS_NATIVE_INSTANCE(rhs) &&
        AS_INSTANCE(rhs)->klass->classType == CLS_STRING)
    {
        StringData *lhs = GET_NATIVE_INSTANCE_DATA(StringData, self);
        StringData *rhs = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
        if (lhs->length != rhs->length)
            return FALSE_VAL;

        if (strncmp(lhs->chars, rhs->chars, lhs->length) == 0)
            return TRUE_VAL;
    }
    return FALSE_VAL;
}

VALUE string_hash(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    return create_number(vm, (double)data->hash);
}

VALUE string_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return self;
}

VALUE string_trim_left(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    utf8proc_int32_t *intermediate = ALLOCATE(utf8proc_int32_t, data->length + 1);
    utf8proc_ssize_t intermediate_length = utf8proc_decompose(
        (const utf8proc_uint8_t *)data->chars, data->length,
        intermediate, data->length,
        UTF8PROC_NULLTERM);
    if (intermediate_length < 0)
    {
        FREE_ARRAY(utf8proc_int32_t, intermediate, data->length - 1);
        throw_exception_native(vm, "Exception", "ERROR: %s\n", utf8proc_errmsg(intermediate_length));
        return NIL_VAL;
    }
    char *output = ALLOCATE(char, data->length + 1);
    bool found_non_whitespace = false;
    int output_offset = 0;

    for (int i = 0; i < intermediate_length; i++)
    {
        if (!found_non_whitespace)
        {
            const utf8proc_property_t *prop = utf8proc_get_property(intermediate[i]);
            if (prop->bidi_class != UTF8PROC_BIDI_CLASS_WS)
                found_non_whitespace = true;
        }
        if (found_non_whitespace)
        {
            output_offset += utf8proc_encode_char(intermediate[i], (utf8proc_uint8_t *) &output[output_offset]);
        }
    }
    VALUE result = copyString(vm, (const char*)output, output_offset);
    FREE_ARRAY(utf8proc_int32_t, intermediate, data->length - 1);
    FREE_ARRAY(char, output, data->length + 1);
    return result;
}

VALUE string_trim_right(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    utf8proc_int32_t* intermediate = ALLOCATE(utf8proc_int32_t, data->length + 1);
    utf8proc_ssize_t intermediate_length = utf8proc_decompose(
        (const utf8proc_uint8_t *)data->chars, data->length,
        intermediate, data->length,
        UTF8PROC_NULLTERM);
    if (intermediate_length < 0)
    {
        FREE_ARRAY(utf8proc_int32_t, intermediate, data->length - 1);
        throw_exception_native(vm, "Exception", "ERROR: %s\n", utf8proc_errmsg(intermediate_length));
        return NIL_VAL;
    }
    char* output = ALLOCATE(char, data->length + 1);

    int i = intermediate_length - 1;
    for (; i >= 0; i--)
    {
        const utf8proc_property_t *prop = utf8proc_get_property(intermediate[i]);
        if (prop->bidi_class != UTF8PROC_BIDI_CLASS_WS)
            break;
    }
    int output_offset = 0;
    for (int j = 0; j <= i; j++)
    {
        output_offset += utf8proc_encode_char(intermediate[j], (utf8proc_uint8_t *) &output[output_offset]);
    }

    VALUE result = copyString(vm, (const char*)output, output_offset);
    FREE_ARRAY(utf8proc_int32_t, intermediate, data->length - 1);
    FREE_ARRAY(char, output, data->length + 1);
    return result;
}

VALUE string_trim(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    if (arg_count == 0)
    {
        VALUE str = string_trim_left(vm, self, 0, NULL);
        return string_trim_right(vm, str, 0, NULL);
    }
    return NIL_VAL;
}

VALUE string_find(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_split(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    VALUE list = list_create(vm);
    push(vm, list);
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    StringData *separator = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
    const char *previous = data->chars;
    const char *string = strstr(data->chars, separator->chars);
    int offset = 0;
    while (string != NULL)
    {
        if (string > previous)
        {
            int length = string - previous;
            VALUE part = copyString(vm, previous, length);
            list_add(vm, list, 1, &part);
            offset += length + separator->length;
        }
        else
        {
            offset++;
        }
        previous = string + separator->length;
        string = strstr(&data->chars[offset], separator->chars);
    }
    VALUE part = copyString(vm, previous, data->length - offset);
    list_add(vm, list, 1, &part);
    pop(vm);
    return list;
}

VALUE string_replace(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE string_starts_with_q(VM UNUSED(*vm), VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        StringData *lhs = GET_NATIVE_INSTANCE_DATA(StringData, self);
        StringData *rhs = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
        if (rhs->length > lhs->length)
            return FALSE_VAL;

        if (memcmp(lhs->chars, rhs->chars, rhs->length) == 0)
        {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
}

VALUE string_ends_with_q(VM UNUSED(*vm), VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        StringData *lhs = GET_NATIVE_INSTANCE_DATA(StringData, self);
        StringData *rhs = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
        if (rhs->length > lhs->length)
            return FALSE_VAL;

        if (memcmp(&lhs->chars[lhs->length - rhs->length], rhs->chars, rhs->length) == 0)
        {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
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
        (utf8proc_uint8_t **)&dest_string,
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
        (utf8proc_uint8_t **)&dest_string,
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
            (const utf8proc_uint8_t *)&data->chars[offset], remaining, &current_codepoint);
        if (bytes_read == -1)
            break;
        offset += bytes_read;
        remaining -= bytes_read;
        length++;
    }
    return create_number(vm, (double)length);
}

VALUE string_concatenate(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        StringData *lhs = GET_NATIVE_INSTANCE_DATA(StringData, self);
        StringData *rhs = GET_NATIVE_INSTANCE_DATA(StringData, arguments[0]);
        int new_len = (lhs->length + rhs->length) + 1;
        char *new_string = ALLOCATE(char, new_len);
        memcpy(new_string, lhs->chars, lhs->length);
        memcpy(&new_string[lhs->length], rhs->chars, rhs->length);
        new_string[new_len - 1] = 0;
        return takeString(vm, new_string, new_len);
    }
    return NIL_VAL;
}

VALUE string_get_at(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    uint64_t index = (int64_t) number_get_value(arguments[0]);
    StringData *data = GET_NATIVE_INSTANCE_DATA(StringData, self);
    utf8proc_int32_t current_codepoint;
    utf8proc_ssize_t remaining = data->length;
    size_t offset = 0;
    size_t length = 0;
    while (offset < data->length)
    {
        utf8proc_ssize_t bytes_read = utf8proc_iterate(
            (const utf8proc_uint8_t *)&data->chars[offset], remaining, &current_codepoint);
        if (bytes_read == -1)
            break;
        offset += bytes_read;
        remaining -= bytes_read;
        if (length == index)
        {
            char *temp = ALLOCATE(char, 5);
            utf8proc_ssize_t result_len = utf8proc_encode_char(current_codepoint, (utf8proc_uint8_t *)temp);
            temp = reallocate(temp, 5, result_len + 1);
            temp[result_len] = '\0';
            return string_create(vm, temp, result_len);
        }
        length++;
    }
    throw_exception_native(vm, "IndexOutOfBoundsException", "%ld was not a valid string index", index);
    return NIL_VAL;
}

void init_string(VM *vm, VALUE obj_klass)
{
    string_class = bootstrapNativeClass(
        vm, "String",
        string_constructor, string_destructor,
        CLS_STRING, sizeof(StringData), true);
    init_object(vm, obj_klass);
    completeNativeClassDefinition(vm, obj_klass, NULL);
    complete_iterable(vm);
    completeNativeClassDefinition(vm, string_class, "Iterable");
    defineNativeMethod(vm, string_class, &string_trim_left, "left_trim", 0, false);
    defineNativeMethod(vm, string_class, &string_trim_right, "right_trim", 0, false);
    defineNativeMethod(vm, string_class, &string_trim, "trim", 0, false);
    defineNativeMethod(vm, string_class, &string_find, "find", 1, false);
    defineNativeMethod(vm, string_class, &string_split, "split", 1, false);
    defineNativeMethod(vm, string_class, &string_replace, "replace", 2, false);
    defineNativeMethod(vm, string_class, &string_starts_with_q, "starts_with?", 1, false);
    defineNativeMethod(vm, string_class, &string_ends_with_q, "ends_with?", 1, false);
    defineNativeMethod(vm, string_class, &string_empty_q, "empty?", 0, false);
    defineNativeMethod(vm, string_class, &string_to_lower, "to_lower", 0, false);
    defineNativeMethod(vm, string_class, &string_to_upper, "to_upper", 0, false);
    defineNativeMethod(vm, string_class, &string_to_string, "to_string", 0, false);
    defineNativeMethod(vm, string_class, &string_length, "length", 0, false);
    defineNativeMethod(vm, string_class, &string_length, "count", 0, false);
    defineNativeMethod(vm, string_class, &string_iterator, "iterator", 0, false);
    defineNativeOperator(vm, string_class, &string_concatenate, 1, OPERATOR_PLUS);
    defineNativeOperator(vm, string_class, &string_equals, 1, OPERATOR_EQUALS);
    defineNativeOperator(vm, string_class, &string_get_at, 1, OPERATOR_INDEX);

    string_iterator_class = defineNativeClass(
        vm, "StringIterator",
        &string_iterator_constructor,
        NULL,
        "Iterator", CLS_ITERATOR, sizeof(StringIterator), true);
    defineNativeMethod(vm, string_iterator_class, &string_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, string_iterator_class, &string_iterator_get_next, "get_next", 0, false);
}
