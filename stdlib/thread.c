#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "comet.h"
#include "comet_stdlib.h"

typedef struct {
    pthread_t thread_id;
    VALUE self;
    VALUE start_routine;
    VALUE arg;
} ThreadData;

void *thread_constructor(void)
{
    ThreadData *data = ALLOCATE(ThreadData, 1);
    data->start_routine = NIL_VAL;
    data->arg = NIL_VAL;
    data->self = NIL_VAL;
    data->thread_id = -1;
    return data;
}

void thread_destructor(void UNUSED(*data))
{
    FREE(ThreadData, data);
}

void *thread_runner(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    return AS_OBJ(call_function(data->self, data->start_routine, 1, &data->arg));
}

VALUE thread_start(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ThreadData *data = GET_NATIVE_INSTANCE_DATA(ThreadData, self);
    data->self = self;
    data->start_routine = arguments[0];
    data->arg = arguments[1];
    pthread_attr_t thread_attributes;
    pthread_attr_init(&thread_attributes);
    int status = pthread_create(&data->thread_id, &thread_attributes, &thread_runner, data);
    if (status != 0)
    {
        throw_exception_native(vm, "SocketException", "Unable to start thread");
    }
    pthread_attr_destroy(&thread_attributes);
    return NIL_VAL;
}

VALUE thread_join(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ThreadData *data = GET_NATIVE_INSTANCE_DATA(ThreadData, self);
    void *result = NULL;
    int status = pthread_join(data->thread_id, &result);
    if (status != 0)
    {
        throw_exception_native(vm, "SocketException", "Unable to join thread");
    }
    if (result != NULL)
        return OBJ_VAL(result);
    return NIL_VAL;
}

void thread_mark_contents(VALUE self)
{
    ThreadData *data = GET_NATIVE_INSTANCE_DATA(ThreadData, self);
    markValue(data->start_routine);
    markValue(data->arg);
}

void init_thread(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Thread", thread_constructor, thread_destructor, NULL, CLS_THREAD);
    defineNativeMethod(vm, klass, &thread_start, "start", 2, false);
    defineNativeMethod(vm, klass, &thread_join, "join", 0, false);
}