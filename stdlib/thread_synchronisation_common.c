#include "cometlib.h"
#include "thread_sync_common.h"
#include "comet.h"

void init_thread_sync(VM *vm)
{
    VALUE cond_var_class = defineNativeClass(
        vm, "ConditionVariable", &cond_var_constructor, &cond_var_destructor, NULL, NULL, CLS_COND_VAR, sizeof(CondVarData), false);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_one, "signal_one", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_signal_all, "signal_all", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_wait, "wait", 0, false);
    defineNativeMethod(vm, cond_var_class, &cond_var_timed_wait, "timed_wait", 1, false);

    VALUE mutex_class = defineNativeClass(vm, "Mutex", &mutex_constructor, &mutex_destructor, NULL, NULL, CLS_MUTEX, sizeof(MutexData), false);
    defineNativeMethod(vm, mutex_class, &mutex_lock, "lock", 0, false);
    defineNativeMethod(vm, mutex_class, &mutex_timed_lock, "timed_lock", 1, false);
    defineNativeMethod(vm, mutex_class, &mutex_unlock, "unlock", 0, false);
    defineNativeMethod(vm, mutex_class, &mutex_try_lock, "try_lock", 0, false);
}
