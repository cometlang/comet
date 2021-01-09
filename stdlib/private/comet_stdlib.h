#ifndef _COMET_STDLIB_H_
#define _COMET_STDLIB_H_
#include "comet.h"

#define GET_NATIVE_INSTANCE_DATA(type_, self) ((type_ *) AS_NATIVE_INSTANCE(self)->data)

#endif
