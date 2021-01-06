#include "native.h"
#include "object.h"
#include "vm.h"

#include <string.h>
#include <stdio.h>

void defineNativeFunction(const char *name, NativeFn function)
{
    push(OBJ_VAL(newNative(function)));
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    addGlobal(AS_STRING(peek(0)), peek(1));
    pop();
    pop();
}

VALUE defineNativeClass(const char *name, NativeConstructor constructor, NativeDestructor destructor, const char *super_name)
{
    ObjString *name_string = copyString(name, strlen(name));
    push(OBJ_VAL(name_string));
    push(OBJ_VAL(newNativeClass(name_string, constructor, destructor)));
    ObjClass *klass = AS_CLASS(peek(0));
    if (strncmp(name_string->chars, "Object", name_string->length) != 0)
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

        tableAddAll(&(AS_CLASS(parent)->methods), &klass->methods);
        tableAddAll(&(AS_CLASS(parent)->staticMethods), &klass->staticMethods);
        klass->super_ = AS_CLASS(parent);
    }
    if (addGlobal(name_string, peek(0)))
    {
        VALUE result = pop();
        pop();
        return result;
    }
    runtimeError("Redefining class %s", name);
    return NIL_VAL;
}

void defineNativeMethod(VALUE klass, NativeMethod function, const char *name, bool isStatic)
{
    ObjString *name_string = copyString(name, strlen(name));
    push(OBJ_VAL(name_string));
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
    ObjString *name_string = copyString(property_name, strlen(property_name));
    push(OBJ_VAL(name_string));
    tableSet(&AS_INSTANCE(self)->fields, name_string, value);
    pop();
    pop();
    pop();
}

VALUE getNativeProperty(VALUE self, const char *property_name)
{
    Value value;
    ObjString *name_string = copyString(property_name, strlen(property_name));
    if (tableGet(&AS_INSTANCE(self)->fields, name_string, &value))
    {
        return value;
    }
    return NIL_VAL;
}
