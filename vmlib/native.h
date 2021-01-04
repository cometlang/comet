#ifndef _NATIVE_API_H_
#define _NATIVE_API_H_

#include "object.h"

void defineNative(const char *name, NativeFn function);
VALUE defineNativeClass(const char *name, NativeConstructor constructor, NativeDestructor destructor, const char *super);
void defineNativeMethod(VALUE klass, NativeMethod function, const char *name, bool isStatic);
void defineNativeOperator(VALUE klass, NativeMethod function, OPERATOR operator);
void setNativeProperty(VALUE self, const char *property_name, VALUE value);
VALUE getNativeProperty(VALUE self, const char *property);

#endif
