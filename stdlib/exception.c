#include <string.h>
#include "comet.h"
#include "cometlib.h"
#include "native.h"

VALUE exception_init(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1 && IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_STRING))
    {
        setNativeProperty(vm, self, "_message", arguments[0]);
    }
    return NIL_VAL;
}

VALUE ooce_exception_init(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1 && IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_STRING))
    {
        setNativeProperty(vm, self, "_message", arguments[0]);
    }
    else
    {
        VALUE message = copyString(vm, "++?????++ Redo from start", 25);
        push(vm, message);
        setNativeProperty(vm, self, "_message", message);
        pop(vm);
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

VALUE argument_exception_throw_if_nil(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE *arguments)
{
    if (arguments[0] == NIL_VAL)
    {
        throw_exception_native(vm, "ArgumentException", "Argument cannot be nil");
    }
    return NIL_VAL;
}

VALUE argument_exception_throw_if_empty(VM* vm, VALUE klass, int UNUSED(arg_count), VALUE* arguments)
{
    call_function(vm, arguments[0], common_strings[STRING_EMPTY_Q], 0, NULL);
    if (pop(vm) == TRUE_VAL)
    {
        throw_exception_native(vm, "ArgumentException", "Argument cannot be empty");
    }
    return NIL_VAL;
}

VALUE argument_exception_throw_if_nil_or_empty(VM* vm, VALUE klass, int arg_count, VALUE* arguments)
{
    if (arguments[0] == NIL_VAL)
    {
        throw_exception_native(vm, "ArgumentException", "Argument cannot be nil");
        return NIL_VAL;
    }
    return argument_exception_throw_if_empty(vm, klass, arg_count, arguments);
}

VALUE argument_exception_throw_if_nil_or_whitespace(VM* vm, VALUE klass, int UNUSED(arg_count), VALUE* arguments)
{
    if (arguments[0] == NIL_VAL)
    {
        throw_exception_native(vm, "ArgumentException", "Argument cannot be nil");
    }
    else if (IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_STRING))
    {
        call_function(vm, arguments[0], common_strings[STRING_EMPTY_Q], 0, NULL);
        if (pop(vm) == TRUE_VAL)
        {
            throw_exception_native(vm, "ArgumentException", "Argument cannot be empty");
            return NIL_VAL;
        }
        call_function(vm, arguments[0], copyString(vm, "whitespace?", 11), 0, NULL);
        if (pop(vm) == TRUE_VAL)
        {
            throw_exception_native(vm, "ArgumentException", "Argument cannot be whitespace");
            return NIL_VAL;
        }
    }
    else
    {
        throw_exception_native(vm, "ArgumentException", "Cannot check for whitespace with a non-string argument, got %s", getClassNameFromInstance(arguments[0]));
    }

    return NIL_VAL;
}

void init_exception(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Exception", NULL, NULL, NULL, NULL, CLS_EXCEPTION, 0, false);
    defineNativeMethod(vm, klass, &exception_init, "init", 0, false);
    defineNativeMethod(vm, klass, &exception_get_message, "message", 0, false);

    defineNativeClass(vm, "AssertionException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    VALUE argExKlass = defineNativeClass(vm, "ArgumentException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeMethod(vm, argExKlass, &argument_exception_throw_if_nil, "throw_if_nil", 1, true);
    defineNativeMethod(vm, argExKlass, &argument_exception_throw_if_nil_or_whitespace, "throw_if_nil_or_whitespace", 1, true);
    defineNativeMethod(vm, argExKlass, &argument_exception_throw_if_nil_or_empty, "throw_if_nil_or_empty", 1, true);
    defineNativeMethod(vm, argExKlass, &argument_exception_throw_if_empty, "throw_if_empty", 1, true);

    defineNativeClass(vm, "IOException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "SocketException", NULL, NULL, NULL, "IOException", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "TimeoutException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "KeyNotFoundException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "IndexOutOfBoundsException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "MethodNotFoundException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "ImportException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "ThreadException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "FormatException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "InvokeException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeClass(vm, "ParseException", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);

    VALUE ooceKlass = defineNativeClass(vm, "OutOfCheeseError", NULL, NULL, NULL, "Exception", CLS_EXCEPTION, 0, false);
    defineNativeMethod(vm, ooceKlass, &ooce_exception_init, "init", 0, false);
}
