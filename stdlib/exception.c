#include "comet.h"
#include "cometlib.h"
#include "native.h"

VALUE exception_init(VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        setNativeProperty(self, "_message", arguments[0]);
    }
    return NIL_VAL;
}

VALUE exception_get_message(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return getNativeProperty(self, "_message");
}

void init_exception(VM UNUSED(*vm))
{
    VALUE klass = defineNativeClass("Exception", NULL, NULL, NULL);
    defineNativeMethod(klass, &exception_init, "init", false);
    defineNativeMethod(klass, &exception_get_message, "message", false);
}
