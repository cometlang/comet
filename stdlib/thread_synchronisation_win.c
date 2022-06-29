#include <errno.h>
#include <math.h>
#include <time.h>
#include "comet_stdlib.h"
#include "cometlib.h"
#include "thread_sync_common.h"
#include <Windows.h>

void cond_var_constructor(void *instanceData)
{
    CondVarData* data = (CondVarData *)instanceData;
    InitializeConditionVariable(&data->cond_var);
    InitializeCriticalSection(&data->critical_section);
}

void cond_var_destructor(void* data)
{
    CondVarData* cond_var = (CondVarData*)data;
    DeleteCriticalSection(&cond_var->critical_section);
}

VALUE cond_var_signal_one(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE cond_var_signal_all(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE cond_var_wait(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE cond_var_timed_wait(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }

void mutex_constructor(void *instanceData)
{
    MutexData* data = (MutexData *)instanceData;
    data->mutex = CreateMutex(NULL, FALSE, NULL);
}

void mutex_destructor(void* data)
{
    MutexData* cond_var = (MutexData*)data;
    CloseHandle(cond_var->mutex);
}

VALUE mutex_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData* data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    WaitForSingleObject(data->mutex, INFINITE);
    return NIL_VAL;
}

VALUE mutex_timed_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData* data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    DWORD wait_time = (DWORD)number_get_value(arguments[0]) * MILLI_SECONDS_PER_SECOND;
    if (WaitForSingleObject(data->mutex, wait_time) == WAIT_TIMEOUT)
        throw_exception_native(vm, "TimeoutException", "Interval elapsed");
    return NIL_VAL;
}

VALUE mutex_unlock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData* data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    if (!ReleaseMutex(data->mutex))
    {
        throw_exception_native(vm, "Exception", "Could not unlock the mutex");
    }
    return NIL_VAL;
}

VALUE mutex_try_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    MutexData* data = GET_NATIVE_INSTANCE_DATA(MutexData, self);
    if (WaitForSingleObject(data->mutex, 0L) == WAIT_OBJECT_0)
        return TRUE_VAL;
    return FALSE_VAL;
}
