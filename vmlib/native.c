#include "native.h"
#include "vm.h"
#include "comet.h"

#include <string.h>
#include <stdio.h>

void defineNativeFunction(const char *name, NativeFn function)
{
    push(OBJ_VAL(newNativeFunction(function)));
    push(copyString(name, (int)strlen(name)));
    addGlobal(peek(0), peek(1));
    pop();
    pop();
}

VALUE bootstrapNativeClass(const char *name, NativeConstructor constructor, NativeDestructor destructor)
{
    return OBJ_VAL(newNativeClass(name, constructor, destructor));
}

VALUE completeNativeClassDefinition(VALUE klass_, const char *super_name)
{
    ObjClass *klass = AS_CLASS(klass_);
    Value name_string = copyString(klass->name, strlen(klass->name));
    push(name_string);
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
        pop(); // name_string
        pop(); // klass
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
    push(name_string);
    push(klass);
    push(OBJ_VAL(newNativeMethod(klass, function, isStatic)));
    defineMethod(name_string, isStatic);
    pop();
    pop();
}

void defineNativeOperator(VALUE klass, NativeMethod function, OPERATOR operator)
{
    push(klass);
    push(OBJ_VAL(newNativeMethod(klass, function, false)));
    defineOperator(operator);
    pop();
}

void setNativeProperty(VALUE self, const char *property_name, VALUE value)
{
    push(self);
    push(value);
    Value name_string = copyString(property_name, strlen(property_name));
    push(name_string);
    tableSet(&AS_INSTANCE(self)->fields, name_string, value);
    pop();
    pop();
    pop();
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
