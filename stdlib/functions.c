#include "cometlib.h"
#include "comet.h"

#include <time.h>

static Value clockNative(int UNUSED(argCount), Value UNUSED(*args))
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void init_functions(void)
{
    defineNativeFunction("clock", &clockNative);
}
