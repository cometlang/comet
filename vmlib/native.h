#ifndef _NATIVE_API_H_
#define _NATIVE_API_H_

#include "objects.h"
#include "vm.h"

void defineNativeFunction(VM *vm, const char *name, NativeFn function);
VALUE defineNativeClass(
    VM *vm,
    const char *name,
    NativeConstructor constructor,
    NativeDestructor destructor,
    MarkNativeObject marker,
    const char *super_name,
    ClassType classType,
    size_t dataSize,
    bool final);
void defineNativeMethod(VM *vm, VALUE klass, NativeMethod function, const char *name, uint8_t arity, bool isStatic);
void defineNativeOperator(VM *vm, VALUE klass, NativeMethod function, uint8_t arity, OPERATOR operator_);
void setNativeProperty(VM *vm, VALUE self, const char *property_name, VALUE value);
VALUE getNativeProperty(VM *vm, VALUE self, const char *property_name);


VALUE bootstrapNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor, ClassType classType, size_t dataSize, bool final);
VALUE completeNativeClassDefinition(VM *vm, VALUE klass_, const char *super_name);
#endif
