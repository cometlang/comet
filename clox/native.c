#include "native.h"
#include "object.h"
#include "vm.h"

#include <string.h>
#include <stdio.h>

void defineNative(const char *name, NativeFn function)
{
    push(OBJ_VAL(newNative(function)));
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    addGlobal(AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

VALUE defineNativeClass(const char *name, NativeConstructor *constructor, NativeDestructor *destructor, const char UNUSED(*super))
{
    push(OBJ_VAL(copyString(name, strlen(name))));
    push(OBJ_VAL(newNativeClass(AS_STRING(peek(0)), constructor, destructor)));
    if (strcmp(name, "Object") != 0)
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

        tableAddAll(&AS_CLASS(parent)->methods, &AS_CLASS(peek(0))->methods);
    }
    if (addGlobal(AS_STRING(peek(1)), peek(0)))
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
    push(OBJ_VAL(copyString(name, strlen(name))));
    push(klass);
    push(OBJ_VAL(newNativeMethod(klass, function, isStatic)));
    defineMethod(AS_STRING(peek(2)), isStatic);
    pop();
}
