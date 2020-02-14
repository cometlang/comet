#ifndef _COMET_H_
#define _COMET_H_

#include "value.h"
#include "compiler.h"
#include "object.h"

void init_stdlib();

VALUE defineNativeClass(const char *name, NativeConstructor *constructor, NativeDestructor *destructor, const char *super);
void defineNativeMethod(VALUE klass, NativeMethod function, const char *name, bool isStatic);

#endif
