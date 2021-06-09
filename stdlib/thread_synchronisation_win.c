#include <errno.h>
#include <math.h>
#include <time.h>
#include "comet_stdlib.h"
#include "cometlib.h"

typedef struct {
    void* thing;
} CondVarData;

void* cond_var_constructor(void)
{
    CondVarData* data = ALLOCATE(CondVarData, 1);
    return data;
}

void cond_var_destructor(void* data)
{
    CondVarData* cond_var = (CondVarData*)data;
    FREE(CondVarData, data);
}

VALUE cond_var_signal_one(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE cond_var_signal_all(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE cond_var_wait(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE cond_var_timed_wait(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }

typedef struct {
    void* thing;
} MutexData;

void* mutex_constructor(void)
{
    MutexData* data = ALLOCATE(MutexData, 1);
    return data;
}

void mutex_destructor(void* data)
{
    MutexData* cond_var = (MutexData*)data;
    FREE(MutexData, data);
}

VALUE mutex_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE mutex_timed_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE mutex_unlock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }
VALUE mutex_try_lock(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments)) { return NIL_VAL; }

void init_thread_sync(VM* vm)
{
    VALUE cond_var_class = defineNativeClass(
        vm, "ConditionVariable", &cond_var_constructor, &cond_var_destructor, NULL, CLS_COND_VAR);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_one, "signal_one", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_all, "signal_all", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_wait, "wait", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_timed_wait, "timed_wait", 1, false);

    VALUE mutex_class = defineNativeClass(vm, "Mutex", &mutex_constructor, &mutex_destructor, NULL, CLS_MUTEX);
    defineNativeMethod(vm, mutex_class, &mutex_lock, "lock", 0, false);
    defineNativeMethod(vm, mutex_class, &mutex_timed_lock, "timed_lock", 1, false);
    defineNativeMethod(vm, mutex_class, &mutex_unlock, "unlock", 0, false);
    defineNativeMethod(vm, mutex_class, &mutex_try_lock, "try_lock", 0, false);
}