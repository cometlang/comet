#include "cometlib.h"
#include "comet.h"

#include <stdio.h>
#include <time.h>

static Value clockNative(int UNUSED(argCount), Value UNUSED(*args))
{
    // return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
    return NIL_VAL;
}

static VALUE printNative(int arg_count, VALUE *args)
{
    for (int i = 0; i < arg_count; i++)
    {
        VALUE string = call_function(args[i], common_strings[STRING_TO_STRING], 0, NULL);
        printf("%s", string_get_cstr(string));
    }
    printf("\n");
    return NIL_VAL;
}

void init_functions(VM *vm)
{
    defineNativeFunction(vm, "clock", &clockNative);
    defineNativeFunction(vm, "print", &printNative);
}
