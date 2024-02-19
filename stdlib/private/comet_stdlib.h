#ifndef _COMET_STDLIB_H_
#define _COMET_STDLIB_H_
#include "comet.h"

#define GET_NATIVE_INSTANCE_DATA(type_, self) ((type_ *) AS_NATIVE_INSTANCE(self))

#ifdef __cplusplus
extern "C" {
#endif

VALUE enum_create(VM *vm);
void enum_add_value(VM *vm, VALUE enum_instance, const char *name, uint64_t value);
uint64_t enumvalue_get_value(VALUE instance);
VALUE enum_get_from_value(VALUE enum_instance, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif
