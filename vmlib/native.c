#include "native.h"
#include "vm.h"
#include "comet.h"

#include <string.h>
#include <stdio.h>

void defineNativeFunction(const char *name, NativeFn function)
{
    push(&vm, OBJ_VAL(newNativeFunction(function)));
    push(&vm, copyString(name, (int)strlen(name)));
    addGlobal(peek(&vm, 0), peek(&vm, 1));
    pop(&vm);
    pop(&vm);
}

VALUE bootstrapNativeClass(const char *name, NativeConstructor constructor, NativeDestructor destructor)
{
    return OBJ_VAL(newNativeClass(name, constructor, destructor));
}

VALUE completeNativeClassDefinition(VALUE klass_, const char *super_name)
{
    ObjClass *klass = AS_CLASS(klass_);
    Value name_string = copyString(klass->name, strlen(klass->name));
    push(&vm, name_string);
    if (string_compare_to_cstr(name_string, "Object") !=0)
    {
        Value parent;
        if (super_name == NULL)
        {
            super_name = "Object";
        }
        if (!findGlobal(copyString(super_name, strlen(super_name)), &parent))
        {
            runtimeError("Could not inherit from unknown class '%s'", super_name);
            return NIL_VAL;
        }

        ObjClass *parent_class = AS_CLASS(parent);

        tableAddAll(&parent_class->methods, &klass->methods);
        tableAddAll(&parent_class->staticMethods, &klass->staticMethods);
        klass->super_ = AS_CLASS(parent);
    }
    if (addGlobal(name_string, OBJ_VAL(klass)))
    {
        pop(&vm); // name_string
        pop(&vm); // klass
        return OBJ_VAL(klass);
    }
    else
    {
        runtimeError("Redefining class %s", klass->name);
        return NIL_VAL;
    }
}

VALUE defineNativeClass(const char *name, NativeConstructor constructor, NativeDestructor destructor, const char *super_name)
{
    VALUE klass = OBJ_VAL(newNativeClass(name, constructor, destructor));
    return completeNativeClassDefinition(klass, super_name);
}

void defineNativeMethod(VALUE klass, NativeMethod function, const char *name, bool isStatic)
{
    Value name_string = copyString(name, strlen(name));
    push(&vm, name_string);
    push(&vm, klass);
    push(&vm, OBJ_VAL(newNativeMethod(klass, function, isStatic)));
    defineMethod(name_string, isStatic);
    pop(&vm);
    pop(&vm);
}

void defineNativeOperator(VALUE klass, NativeMethod function, OPERATOR operator)
{
    push(&vm, klass);
    push(&vm, OBJ_VAL(newNativeMethod(klass, function, false)));
    defineOperator(operator);
    pop(&vm);
}

void setNativeProperty(VALUE self, const char *property_name, VALUE value)
{
    push(&vm, self);
    push(&vm, value);
    Value name_string = copyString(property_name, strlen(property_name));
    push(&vm, name_string);
    tableSet(&AS_INSTANCE(self)->fields, name_string, value);
    pop(&vm);
    pop(&vm);
    pop(&vm);
}

VALUE getNativeProperty(VALUE self, const char *property_name)
{
    Value value;
    Value name_string = copyString(property_name, strlen(property_name));
    if (tableGet(&AS_INSTANCE(self)->fields, name_string, &value))
    {
        return value;
    }
    return NIL_VAL;
}
