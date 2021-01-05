#ifndef _COMET_H_
#define _COMET_H_

#include "value.h"
#include "compiler.h"
#include "object.h"
#include "native.h"
#include "memory.h"

void init_stdlib();
const char *get_cstr(VALUE self);

#endif
