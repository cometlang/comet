#include "cometlib.h"
#include "comet.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

static Value clockNative(VM *vm, int UNUSED(argCount), Value UNUSED(*args))
{
    return create_number(vm, (double)clock() / CLOCKS_PER_SEC);
}

static VALUE printNative(VM UNUSED(*vm), int arg_count, VALUE *args)
{
    for (int i = 0; i < arg_count; i++)
    {
        VALUE string = call_function(args[i], common_strings[STRING_TO_STRING], 0, NULL);
        printf("%s", string_get_cstr(string));
    }
    printf("\n");
    return NIL_VAL;
}

static VALUE assertNative(VM *vm, int UNUSED(arg_count), VALUE *args)
{
    if (bool_is_falsey(args[0]))
    {
        const char *message = "assert failed";
        if (arg_count == 2)
            message = string_get_cstr(args[1]);
        throw_exception_native(vm, "AssertionException", message);
    }
    return NIL_VAL;
}

VALUE callable_p(VM UNUSED(*vm), int UNUSED(arg_count), VALUE *args)
{
    VALUE val = args[0];
    if (IS_BOUND_METHOD(val) || IS_CLASS(val) || IS_FUNCTION(val) || IS_CLOSURE(val) ||
        IS_NATIVE(val) || IS_NATIVE_METHOD(val) || IS_NATIVE_CLASS(val))
        return TRUE_VAL;

    return FALSE_VAL;
}

VALUE fn_sleep(VM UNUSED(*vm), int UNUSED(arg_count), VALUE *args)
{
    double input_time = number_get_value(args[0]);
#ifdef WIN32
    Sleep(input_time * MILLI_SECONDS_PER_SECOND);
#else
    struct timespec sleep_time, remainder;
    sleep_time.tv_sec = floor(input_time);
    sleep_time.tv_nsec = (input_time - sleep_time.tv_sec) * NANO_SECONDS_PER_SECOND;
    nanosleep(&sleep_time, &remainder);
#endif
    return NIL_VAL;
}

#if DEBUG_TRACE_EXECUTION
VALUE fn_print_stack(VM UNUSED(*vm), int UNUSED(arg_count), VALUE UNUSED(*args))
{
    toggle_stack_printing();
    return NIL_VAL;
}
#endif

void init_functions(VM *vm)
{
    defineNativeFunction(vm, "clock", &clockNative);
    defineNativeFunction(vm, "print", &printNative);
    defineNativeFunction(vm, "assert", &assertNative);
    defineNativeFunction(vm, "callable?", &callable_p);
    defineNativeFunction(vm, "sleep", &fn_sleep);
#if DEBUG_TRACE_EXECUTION
    defineNativeFunction(vm, "toggle_stack_printing", &fn_print_stack);
#endif
}
