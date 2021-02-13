#ifndef _COMET_H_
#define _COMET_H_

#include "value.h"
#include "objects.h"
#include "native.h"
#include "mem.h"

void init_stdlib(VM *vm);

VALUE obj_hash(VM *vm, VALUE self, int arg_count, VALUE *arguments);

VALUE string_create(VM *vm, char *chars, int length);
int string_compare_to_cstr(VALUE self, const char *cstr);
const char *string_get_cstr(VALUE self);
VALUE string_hash(VM *vm, VALUE self, int arg_count, VALUE *arguments);
uint32_t string_hash_cstr(const char *string, int length);

void exception_set_stacktrace(VM *vm, VALUE self, VALUE stacktrace);
VALUE exception_get_stacktrace(VM *vm, VALUE self);
VALUE instanceof(VALUE self, VALUE klass);

bool bool_is_falsey(Value value);

VALUE create_number(VM *vm, double number);
double number_get_value(VALUE self);

VALUE create_list(VM *vm);
VALUE list_add(VM *vm, VALUE self, int arg_count, VALUE *arguments);

#endif
