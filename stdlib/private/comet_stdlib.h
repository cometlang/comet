#ifndef _COMET_STDLIB_H_
#define _COMET_STDLIB_H_
#include "comet.h"

#define GET_NATIVE_INSTANCE_DATA(type_, self) ((type_ *) AS_NATIVE_INSTANCE(self)->data)

typedef struct {
  double num;
} NumberData;

bool is_a_string(VALUE instance);
bool is_a_number(VALUE instance);

#endif
