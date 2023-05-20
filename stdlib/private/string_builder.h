#ifndef _COMET_STDLIB_STRING_BUILDER_H_
#define _COMET_STDLIB_STRING_BUILDER_H_
#include "comet.h"
#include "utf8proc.h"

VALUE create_string_builder(VM *vm);

void string_builder_add_codepoint(VALUE self, utf8proc_int32_t codepoint);

VALUE string_builder_to_string(VM *vm, VALUE self, int arg_count, VALUE *arguments);

void string_builder_add_cstr(VM *vm, VALUE self, const char *cstr);

#endif