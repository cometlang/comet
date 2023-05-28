#include "comet.h"
#include "cometlib.h"

#include <cstdlib>
#include <cerrno>
#include <cstring>

#ifdef WIN32
#include <processenv.h>
#endif

extern "C" {

VALUE env_set_value(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE *arguments)
{
#ifndef WIN32
    if (arguments[1] == NIL_VAL)
    {
        unsetenv(string_get_cstr(arguments[0]));
        return NIL_VAL;
    }
#endif
    if (!isObjOfStdlibClassType(arguments[0], CLS_STRING) ||
        !isObjOfStdlibClassType(arguments[1], CLS_STRING))
    {
        throw_exception_native(vm, "ArgumentException", "Environment variable names and values can only be strings");
        return NIL_VAL;
    }
#ifndef WIN32
    if (setenv(string_get_cstr(arguments[0]), string_get_cstr(arguments[1]), true) != 0)
    {
        throw_exception_native(vm, "Exception", "Could not set environment variable: %s", strerror(errno));
    }
#else
    if (!SetEnvironmentVariableA(string_get_cstr(arguments[0]), string_get_cstr(arguments[1])))
    {
        throw_exception_native(vm, "Exception", "Could not set environment variable: %s", strerror(errno));
    }
#endif
    return NIL_VAL;
}

VALUE env_get_value(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE *arguments)
{
    const char *env_var = string_get_cstr(arguments[0]);
    char *value = std::getenv(env_var);
    if (value == NULL)
    {
        throw_exception_native(vm, "Exception", "Environment variable %s did not exist", env_var);
        return NIL_VAL;
    }

    return copyString(vm, value, strlen(value));
}

void init_env(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "EnvVars", NULL, NULL, NULL, NULL, CLS_ENV, 0, false);
    defineNativeOperator(vm, klass, &env_set_value, 2, OPERATOR_INDEX_ASSIGN);
    defineNativeOperator(vm, klass, &env_get_value, 1, OPERATOR_INDEX);
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(klass)));
    push(vm, instance);
    addGlobal(copyString(vm, "ENV", 3), instance);
    pop(vm);
}

}