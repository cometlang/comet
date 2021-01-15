#ifndef _NATIVE_API_H_
#define _NATIVE_API_H_

#include "objects.h"
#include "vm.h"

void defineNativeFunction(VM *vm, const char *name, NativeFn function);
VALUE defineNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor, const char *super_name);
void defineNativeMethod(VM *vm, VALUE klass, NativeMethod function, const char *name, bool isStatic);
void defineNativeOperator(VM *vm, VALUE klass, NativeMethod function, OPERATOR operator);
void setNativeProperty(VM *vm, VALUE self, const char *property_name, VALUE value);
VALUE getNativeProperty(VM *vm, VALUE self, const char *property_name);


VALUE bootstrapNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor);
VALUE completeNativeClassDefinition(VM *vm, VALUE klass_, const char *super_name);
#endif
