#define __GNU_SOURCE
#include <pthread.h>
#include "comet_stdlib.h"

typedef struct {
    pthread_cond_t cond_var;
} CondVarData;

void *cond_var_constructor(void)
{
    CondVarData *data = ALLOCATE(CondVarData, 1);
    pthread_cond_init(&data->cond_var, NULL);
    return data;
}

void cond_var_destructor(void *data)
{
    CondVarData *cond_var = (CondVarData *)data;
    pthread_cond_destroy(&cond_var->cond_var);
    FREE(CondVarData, data);
}

VALUE cond_var_signal_one(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE cond_var_signal_all(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE cond_var_wait(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE cond_var_timed_wait(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_thread_sync(VM *vm)
{
    VALUE cond_var_class = defineNativeClass(
        vm, "ConditionVariable", &cond_var_constructor, &cond_var_destructor, NULL, CLS_COND_VAR);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_one, "signal_one", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_all, "signal_all", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_wait, "wait", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_timed_wait, "timed_wait", 0, false);
}
