#include <string.h>
#include "comet.h"
#include "cometlib.h"
#include "native.h"

VALUE exception_init(VM *vm, VALUE UNUSED(self), int arg_count, VALUE UNUSED(*arguments))
{
    if (arg_count == 1)
    {
        setNativeProperty(vm, self, "_message", arguments[0]);
    }
    return NIL_VAL;
}

VALUE exception_get_message(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return getNativeProperty(vm, self, "_message");
}

void exception_set_stacktrace(VM *vm, VALUE self, VALUE stacktrace)
{
    setNativeProperty(vm, self, "stacktrace", stacktrace);
}

VALUE exception_get_stacktrace(VM *vm, VALUE self)
{
    return getNativeProperty(vm, self, "stacktrace");
}

void init_exception(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Exception", NULL, NULL, NULL, CLS_EXCEPTION, 0, false);
    defineNativeMethod(vm, klass, &exception_init, "init", 1, false);
    defineNativeMethod(vm, klass, &exception_get_message, "message", 0, false);

    defineNativeClass(vm, "AssertionException", NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "ArgumentException", NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "IOException", NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "SocketException", NULL, NULL, "IOException", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "TimeoutException", NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "KeyNotFoundException", NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "IndexOutOfBoundsException", NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
}
