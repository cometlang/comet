#include "cometlib.h"
#include "comet.h"

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
        VALUE string;
        VM *func_vm = call_function(args[i], common_strings[STRING_TO_STRING], 0, NULL, &string);
        printf("%s", string_get_cstr(string));
        freeVM(func_vm);
        FREE(VM, func_vm);
    }
    printf("\n");
    return NIL_VAL;
}

static VALUE assertNative(VM *vm, int UNUSED(arg_count), VALUE *args)
{
    if (bool_is_falsey(args[0]))
    {
        throw_exception_native(vm, "AssertionException", "assert failed");
    }
    return NIL_VAL;
}

void init_functions(VM *vm)
{
    defineNativeFunction(vm, "clock", &clockNative);
    defineNativeFunction(vm, "print", &printNative);
    defineNativeFunction(vm, "assert", &assertNative);
}
