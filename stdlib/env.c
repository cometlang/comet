#include "comet.h"
#include "cometlib.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>

VALUE env_set_value(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE *arguments)
{
#ifndef WIN32
    if (setenv(string_get_cstr(arguments[0]), string_get_cstr(arguments[1]), true) != 0)
    {
        throw_exception_native(vm, "Exception", "Could not set environment variable: %s", strerror(errno));
    }
#endif
    return NIL_VAL;
}

VALUE env_get_value(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE *arguments)
{
    const char *env_var = string_get_cstr(arguments[0]);
    char *value = getenv(env_var);
    if (value == NULL)
    {
        throw_exception_native(vm, "Exception", "Environment variable %s did not exist", env_var);
        return NIL_VAL;
    }

    return copyString(vm, value, strlen(value));
}

void init_env(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "EnvVars", NULL, NULL, NULL, CLS_ENV);
    defineNativeOperator(vm, klass, &env_set_value, 2, OPERATOR_INDEX_ASSIGN);
    defineNativeOperator(vm, klass, &env_get_value, 1, OPERATOR_INDEX);
    Obj *instance = newInstance(vm, AS_CLASS(klass));
    addGlobal(copyString(vm, "ENV", 3), OBJ_VAL(instance));
}