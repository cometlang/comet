#include "native.h"
#include "object.h"
#include "vm.h"

#include <string.h>
#include <stdio.h>

void defineNative(const char *name, NativeFn function)
{
    push(OBJ_VAL(newNative(function)));
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    addGlobal(AS_STRING(peek(0)), peek(1));
    pop();
    pop();
}

VALUE defineNativeClass(const char *name, NativeConstructor *constructor, NativeDestructor *destructor, const char UNUSED(*super))
{
    ObjString *name_string = copyString(name, strlen(name));
    push(OBJ_VAL(name_string));
    push(OBJ_VAL(newNativeClass(name_string, constructor, destructor)));
    if (strncmp(name_string->chars, "Object", name_string->length) != 0)
    {
        Value parent;
        if (super == NULL)
        {
            super = "Object";
        }
        if (!findGlobal(copyString(super, strlen(super)), &parent))
        {
            runtimeError("Could not inherit from unknown class '%s'", super);
            return NIL_VAL;
        }

        tableAddAll(&(AS_CLASS(parent)->methods), &AS_CLASS(peek(0))->methods);
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
}
