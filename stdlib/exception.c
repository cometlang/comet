#include "comet.h"
#include "cometlib.h"
#include "native.h"

VALUE exception_init(VALUE UNUSED(self), int arg_count, VALUE UNUSED(*arguments))
{
    if (arg_count == 1)
    {
        // setNativeProperty(self, "_message", arguments[0]);
    }
    return NIL_VAL;
}

VALUE exception_get_message(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return getNativeProperty(self, "_message");
}

void init_exception(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Exception", NULL, NULL, NULL);
    defineNativeMethod(vm, klass, &exception_init, "init", false);
    defineNativeMethod(vm, klass, &exception_get_message, "message", false);
}
