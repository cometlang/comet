#ifndef COMET_STDLIB_THREAD_SYNC_COMMON_H_
#define COMET_STDLIB_THREAD_SYNC_COMMON_H_
#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "comet_stdlib.h"

typedef struct {
    ObjInstance obj;
#ifdef WIN32
    CONDITION_VARIABLE cond_var;
    CRITICAL_SECTION critical_section;
#else
    pthread_cond_t cond_var;
    pthread_mutex_t lock;
#endif
} CondVarData;

void cond_var_constructor(void *instanceData);
void cond_var_destructor(void *data);
VALUE cond_var_signal_one(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE cond_var_signal_all(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE cond_var_wait(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE cond_var_timed_wait(VM *vm, VALUE self, int arg_count, VALUE *arguments);

typedef struct {
    ObjInstance obj;
#ifdef WIN32
    HANDLE mutex;
#else
    pthread_mutex_t mutex;
#endif
} MutexData;

void mutex_constructor(void *instanceData);
void mutex_destructor(void *data);
VALUE mutex_unlock(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE mutex_lock(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE mutex_try_lock(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE mutex_timed_lock(VM *vm, VALUE self, int arg_count, VALUE *arguments);

#endif