#ifndef _COMET_H_
#define _COMET_H_

#include "value.h"
#include "compiler.h"
#include "objects.h"
#include "native.h"
#include "mem.h"

void init_stdlib(VM *vm);

VALUE obj_hash(VM *vm, VALUE self, int arg_count, VALUE *arguments);

void *string_set_cstr(ObjNativeInstance *instance, char *string, int length);
int string_compare_to_cstr(VALUE self, const char *cstr);
const char *string_get_cstr(VALUE self);
VALUE string_hash(VM *vm, VALUE self, int arg_count, VALUE *arguments);
uint32_t string_hash_cstr(const char *string, int length);

void exception_set_stacktrace(VM *vm, VALUE self, VALUE stacktrace);
VALUE instanceof(VALUE self, VALUE klass);

bool bool_is_falsey(Value value);

#endif
