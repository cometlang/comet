#include "utf8proc.h"

#include "cometlib.h"
#include "comet_stdlib.h"
#include "comet_string.h"
#include "string_builder.h"

#define BASE_CODEPOINT_ALLOCATION 16

typedef struct {
    ObjInstance obj;
    utf8proc_int32_t *codepoints;
    size_t size;
    size_t index;
} StringBuilderData_t;

static VALUE string_builder_klass;

static void string_builder_constructor(void *data)
{
    StringBuilderData_t *builder = (StringBuilderData_t *)data;
    builder->codepoints = NULL;
    builder->size = 0;
    builder->index = 0;
}

static void string_builder_destructor(void *data)
{
    StringBuilderData_t *builder = (StringBuilderData_t *)data;
    if (builder->codepoints != NULL)
    {
        FREE_ARRAY(StringBuilderData_t, builder->codepoints, builder->size);
        builder->codepoints = NULL;
        builder->size = 0;
        builder->index = 0;
    }
}

void string_builder_add_codepoint(VALUE self, utf8proc_int32_t codepoint)
{
    StringBuilderData_t *data = GET_NATIVE_INSTANCE_DATA(StringBuilderData_t, self);
    if (data->index == data->size)
    {
        size_t new_size = data->size + (BASE_CODEPOINT_ALLOCATION * sizeof(utf8proc_int32_t));
        data->codepoints = GROW_ARRAY(data->codepoints, utf8proc_int32_t, data->size, new_size);
        data->size = new_size;
    }
    data->codepoints[data->index++] = codepoint;
}

VALUE string_builder_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringBuilderData_t *data = GET_NATIVE_INSTANCE_DATA(StringBuilderData_t, self);
    utf8proc_uint8_t temp[4];
    size_t length = 0;
    for (size_t i = 0; i < data->index; i++)
    {
        utf8proc_ssize_t char_size = utf8proc_encode_char(data->codepoints[i], &temp[0]);
        length += char_size;
    }
    char *output = ALLOCATE(char, length + 1);
    int output_offset = 0;
    for (int j = 0; j < data->index; j++)
    {
        output_offset += utf8proc_encode_char(data->codepoints[j], (utf8proc_uint8_t *)&output[output_offset]);
    }
    output[output_offset] = 0;
    return takeString(vm, output, length);
}

VALUE string_builder_append(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    push(vm, string_iterator(vm, arguments[0], 0, NULL));
    StringIterator *iter = GET_NATIVE_INSTANCE_DATA(StringIterator, peek(vm, 0));
    while (string_iter_get_next(iter))
    {
        string_builder_add_codepoint(self, iter->current_codepoint);
    }
    pop(vm);
    return self;
}

void string_builder_add_cstr(VM *vm, VALUE self, const char *cstr)
{
    int offset = 0;
    size_t remaining = strlen(cstr);
    utf8proc_ssize_t bytes_read = 0;
    utf8proc_int32_t codepoint = 0;
    while (bytes_read > 0)
    {
        bytes_read = utf8proc_iterate((const utf8proc_uint8_t *)&cstr[offset], remaining, &codepoint);
        offset += bytes_read;
        remaining -= bytes_read;
        string_builder_add_codepoint(self, codepoint);
    }
}

VALUE string_builder_pop(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    StringBuilderData_t *data = GET_NATIVE_INSTANCE_DATA(StringBuilderData_t, self);
    if (data->index > 0)
    {
        data->index--;
    }
    return NIL_VAL;
}

VALUE create_string_builder(VM *vm)
{
    VALUE builder = OBJ_VAL(newInstance(vm, AS_CLASS(string_builder_klass)));
    push(vm, builder);
    StringBuilderData_t *data = GET_NATIVE_INSTANCE_DATA(StringBuilderData_t, builder);
    string_builder_constructor(data);
    return pop(vm);
}

void init_string_builder(VM *vm)
{
    string_builder_klass = defineNativeClass(
        vm,
        "StringBuilder",
        &string_builder_constructor,
        &string_builder_destructor,
        NULL,
        NULL,
        CLS_STRING_BUILDER,
        sizeof(StringBuilderData_t),
        true);
    defineNativeMethod(vm, string_builder_klass, &string_builder_to_string, "to_string", 0, false);
    defineNativeMethod(vm, string_builder_klass, &string_builder_append, "append", 1, false);
    defineNativeMethod(vm, string_builder_klass, &string_builder_pop, "pop", 0, false);
    
    defineNativeOperator(vm, string_builder_klass, &string_builder_append, 1, OPERATOR_PLUS);
}