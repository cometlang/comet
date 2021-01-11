#ifndef _COMET_H_
#define _COMET_H_

#include "value.h"
#include "compiler.h"
#include "objects.h"
#include "native.h"
#include "mem.h"

void init_stdlib();
const char *get_cstr(VALUE self);
void *string_set_cstr(ObjNativeInstance *instance, const char *string, int length);
int string_compare_to_cstr(VALUE self, const char *cstr);
const char *string_get_cstr(VALUE self);

#endif
