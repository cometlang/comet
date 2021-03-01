#define __GNU_SOURCE
#include <pthread.h>
#include "comet_stdlib.h"

typedef struct {
    pthread_cond_t cond_var;
    pthread_mutex_t lock;
} CondVarData;

void *cond_var_constructor(void)
{
    CondVarData *data = ALLOCATE(CondVarData, 1);
    pthread_cond_init(&data->cond_var, NULL);
    pthread_mutex_init(&data->lock, NULL);
    return data;
}

void cond_var_destructor(void *data)
{
    CondVarData *cond_var = (CondVarData *)data;
    pthread_cond_destroy(&cond_var->cond_var);
    pthread_mutex_destroy(&cond_var->lock);
    FREE(CondVarData, data);
}

VALUE cond_var_signal_one(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    CondVarData *data = GET_NATIVE_INSTANCE_DATA(CondVarData, self);
    pthread_cond_signal(&data->cond_var);
    return NIL_VAL;
}

VALUE cond_var_signal_all(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    CondVarData *data = GET_NATIVE_INSTANCE_DATA(CondVarData, self);
    pthread_cond_broadcast(&data->cond_var);
    return NIL_VAL;
}

VALUE cond_var_wait(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    CondVarData *data = GET_NATIVE_INSTANCE_DATA(CondVarData, self);
    pthread_cond_wait(&data->cond_var, &data->lock);
    return NIL_VAL;
}

VALUE cond_var_timed_wait(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    CondVarData UNUSED(*data) = GET_NATIVE_INSTANCE_DATA(CondVarData, self);
    // pthread_cond_timedwait(&data->cond_var, &data->lock);
    return NIL_VAL;
}


typedef struct {
    pthread_mutex_t mutex;
} MutexData;

void *mutex_constructor(void)
{
    MutexData *data = ALLOCATE(MutexData, 1);
    pthread_mutex_init(&data->mutex, NULL);
    return data;
}

void mutex_destructor(void *data)
{
    pthread_mutex_destroy(&((MutexData *)data)->mutex);
    FREE(MutexData, data);
}

VALUE mutex_unlock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData *data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    pthread_mutex_unlock(&data->mutex);
    return NIL_VAL;
}

VALUE mutex_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData *data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    pthread_mutex_lock(&data->mutex);
    return NIL_VAL;
}

VALUE mutex_try_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData *data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    if (pthread_mutex_trylock(&data->mutex) == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

void init_thread_sync(VM *vm)
{
    VALUE cond_var_class = defineNativeClass(
        vm, "ConditionVariable", &cond_var_constructor, &cond_var_destructor, NULL, CLS_COND_VAR);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_one, "signal_one", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_all, "signal_all", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_wait, "wait", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_timed_wait, "timed_wait", 0, false);

    VALUE mutex_class = defineNativeClass(vm, "Mutex", &mutex_constructor, &mutex_destructor, NULL, CLS_MUTEX);
    defineNativeMethod(vm, mutex_class, &mutex_lock, "lock", 0, false);
    defineNativeMethod(vm, mutex_class, &mutex_unlock, "unlock", 0, false);
    defineNativeMethod(vm, mutex_class, &mutex_try_lock, "try_lock", 0, false);
}
