#include "native.h"
#include "vm.h"
#include "comet.h"

#include <string.h>
#include <stdio.h>

void defineNativeFunction(VM *vm, const char *name, NativeFn function)
{
    push(vm, OBJ_VAL(newNativeFunction(vm, function)));
    push(vm, copyString(vm, name, (int)strlen(name)));
    addGlobal(vm, peek(vm, 0), peek(vm, 1));
    pop(vm);
    pop(vm);
}

VALUE bootstrapNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor)
{
    return OBJ_VAL(newNativeClass(vm, name, constructor, destructor));
}

VALUE completeNativeClassDefinition(VM *vm, VALUE klass_, const char *super_name)
{
    ObjClass *klass = AS_CLASS(klass_);
    Value name_string = copyString(vm, klass->name, strlen(klass->name));
    push(vm, name_string);
    if (string_compare_to_cstr(name_string, "Object") != 0)
    {
        Value parent;
        if (super_name == NULL)
        {
            super_name = "Object";
        }
        if (!findGlobal(vm, copyString(vm, super_name, strlen(super_name)), &parent))
        {
            runtimeError(vm, "Could not inherit from unknown class '%s'", super_name);
            return NIL_VAL;
        }

        ObjClass *parent_class = AS_CLASS(parent);

        tableAddAll(&parent_class->methods, &klass->methods);
        tableAddAll(&parent_class->staticMethods, &klass->staticMethods);
        klass->super_ = AS_CLASS(parent);
    }
    if (addGlobal(vm, name_string, OBJ_VAL(klass)))
    {
        pop(vm);
        pop(vm);
        return OBJ_VAL(klass);
    }
    else
    {
        runtimeError(vm, "Redefining class %s", klass->name);
        return NIL_VAL;
    }
}

VALUE defineNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor, const char *super_name)
{
    VALUE klass = OBJ_VAL(newNativeClass(vm, name, constructor, destructor));
    return completeNativeClassDefinition(vm, klass, super_name);
}

void defineNativeMethod(VM *vm, VALUE klass, NativeMethod function, const char *name, bool isStatic)
{
    Value name_string = copyString(vm, name, strlen(name));
    push(vm, name_string);
    push(vm, klass);
    push(vm, OBJ_VAL(newNativeMethod(vm, function, isStatic)));
    defineMethod(vm, name_string, isStatic);
    pop(vm);
    pop(vm);
}

void defineNativeOperator(VM *vm, VALUE klass, NativeMethod function, OPERATOR operator)
{
    push(vm, klass);
    push(vm, OBJ_VAL(newNativeMethod(vm, function, false)));
    defineOperator(vm, operator);
    pop(vm);
}

void setNativeProperty(VM *vm, VALUE self, const char *property_name, VALUE value)
{
    push(vm, self);
    push(vm, value);
    Value name_string = copyString(vm, property_name, strlen(property_name));
    push(vm, name_string);
    tableSet(&AS_INSTANCE(self)->fields, name_string, value);
    pop(vm);
    pop(vm);
    pop(vm);
}

VALUE getNativeProperty(VM *vm, VALUE self, const char *property_name)
{
    Value value;
    Value name_string = copyString(vm, property_name, strlen(property_name));
    if (tableGet(&AS_INSTANCE(self)->fields, name_string, &value))
    {
        return value;
    }
    return NIL_VAL;
}
