#ifndef _COMET_STDLIB_H_
#define _COMET_STDLIB_H_
#include "comet.h"

#define GET_NATIVE_INSTANCE_DATA(type_, self) ((type_ *) AS_NATIVE_INSTANCE(self)->data)

typedef struct {
  double num;
} NumberData;

bool is_a_string(VALUE instance);
bool is_a_number(VALUE instance);

VALUE enum_create(VM *vm);
void enum_add_value(VM *vm, VALUE enum_instance, const char *name, uint64_t value);
uint64_t enumvalue_get_value(VALUE instance);

#endif
