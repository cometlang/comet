#include "comet.h"

void *thread_constructor(void)
{
    return NULL;
}

void thread_destructor(void UNUSED(*data))
{
}

VALUE thread_init(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE thread_start(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE thread_join(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}


void init_thread(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Thread", thread_constructor, thread_destructor, NULL);
    defineNativeMethod(vm, klass, &thread_init, "init", 0, false);
    defineNativeMethod(vm, klass, &thread_start, "start", 0, false);
    defineNativeMethod(vm, klass, &thread_join, "join", 0, false);
}