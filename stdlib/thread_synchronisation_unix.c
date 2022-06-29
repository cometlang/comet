#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include "comet_stdlib.h"
#include "cometlib.h"
#include "thread_sync_common.h"

void cond_var_constructor(void *instanceData)
{
    CondVarData *data = (CondVarData *)instanceData;
    pthread_cond_init(&data->cond_var, NULL);
    pthread_mutex_init(&data->lock, NULL);
}

void cond_var_destructor(void *data)
{
    CondVarData *cond_var = (CondVarData *)data;
    pthread_cond_destroy(&cond_var->cond_var);
    pthread_mutex_destroy(&cond_var->lock);
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
    pthread_mutex_lock(&data->lock);
    pthread_cond_wait(&data->cond_var, &data->lock);
    pthread_mutex_unlock(&data->lock);
    return NIL_VAL;
}

static void get_wait_time(struct timespec *time, double wait_interval_s)
{
    clock_gettime(CLOCK_REALTIME, time);
    double wait_increment = wait_interval_s;
    long wait_s = floor(wait_increment);
    long wait_ns = (wait_increment - wait_s) * NANO_SECONDS_PER_SECOND;
    time->tv_sec += wait_s;
    time->tv_nsec += wait_ns;
}

VALUE cond_var_timed_wait(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    CondVarData *data = GET_NATIVE_INSTANCE_DATA(CondVarData, self);
    pthread_mutex_lock(&data->lock);
    struct timespec wait_time;
    get_wait_time(&wait_time, number_get_value(arguments[0]));
    int result = pthread_cond_timedwait(&data->cond_var, &data->lock, &wait_time);
    pthread_mutex_unlock(&data->lock);
    if (result == ETIMEDOUT)
    {
        throw_exception_native(vm, "TimeoutException", "Interval elapsed");
    }
    return NIL_VAL;
}

void mutex_constructor(void *instanceData)
{
    MutexData *data = (MutexData *)instanceData;
    pthread_mutex_init(&data->mutex, NULL);
}

void mutex_destructor(void *data)
{
    pthread_mutex_destroy(&((MutexData *)data)->mutex);
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

VALUE mutex_timed_lock(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    MutexData *data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    struct timespec wait_time;
    get_wait_time(&wait_time, number_get_value(arguments[0]));
    int result = pthread_mutex_timedlock(&data->mutex, &wait_time);
    if (result == ETIMEDOUT)
    {
        throw_exception_native(vm, "TimeoutException", "Interval elapsed");
    }
    return NIL_VAL;
}
