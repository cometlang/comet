#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "comet.h"
#include "comet_stdlib.h"
#include "cometlib.h"
#ifdef WIN32
#include <Windows.h>
#include <conio.h>
#include <process.h>
#else
#include <pthread.h>
#endif

typedef struct {
    ObjNativeInstance obj;
#ifdef WIN32
    HANDLE thread_handle;
    DWORD thread_id;
#else
    pthread_t thread_id;
#endif
    VALUE self;
    VALUE start_routine;
    VALUE arg;
} ThreadData;

void thread_constructor(void *instanceData)
{
    ThreadData *data = (ThreadData *)instanceData;
    data->start_routine = NIL_VAL;
    data->arg = NIL_VAL;
    data->self = NIL_VAL;
    data->thread_id = -1;
}

void *thread_runner(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    return (void *)(uintptr_t)call_function(data->self, data->start_routine, 1, &data->arg);
}

VALUE thread_start(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    ThreadData *data = GET_NATIVE_INSTANCE_DATA(ThreadData, self);
    data->self = self;
    data->start_routine = arguments[0];
    data->arg = arg_count > 1 ? arguments[1] : NIL_VAL;
    int status = 0;
    if (callable_p(vm, 1, arguments) == FALSE_VAL)
    {
        throw_exception_native(vm, "ArgumentException", "The first argument to Thread::start was not callable");
        return NIL_VAL;
    }
#ifdef WIN32
    data->thread_handle = CreateThread(NULL, 0, &thread_runner, data, 0, &data->thread_id);
    status = data->thread_handle == NULL ? -1 : 0;
#else
    pthread_attr_t thread_attributes;
    pthread_attr_init(&thread_attributes);
    status = pthread_create(&data->thread_id, &thread_attributes, &thread_runner, data);
#endif
    if (status != 0)
    {
        throw_exception_native(vm, "ThreadException", "Unable to start thread");
    }
#ifndef WIN32
    pthread_attr_destroy(&thread_attributes);
#endif
    return NIL_VAL;
}

VALUE thread_join(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ThreadData *data = GET_NATIVE_INSTANCE_DATA(ThreadData, self);
    void *result = NULL;
    int status = 0;
#ifdef WIN32
    status = WaitForSingleObject(data->thread_handle, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
    CloseHandle(data->thread_handle);
#else
    status = pthread_join(data->thread_id, &result);
#endif
    if (status != 0)
    {
        throw_exception_native(vm, "ThreadException", "Unable to join thread");
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
    VALUE klass = defineNativeClass(vm, "Thread", thread_constructor, NULL, NULL, CLS_THREAD, sizeof(ThreadData), true);
    defineNativeMethod(vm, klass, &thread_start, "start", 1, false);
    defineNativeMethod(vm, klass, &thread_join, "join", 0, false);
}