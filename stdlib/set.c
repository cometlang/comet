#include "comet.h"

void *set_constructor(void)
{
    return NULL;
}

void set_destructor(void UNUSED(*data))
{}

VALUE set_add(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE set_remove(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE set_iterable_empty_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return TRUE_VAL;
}

VALUE set_iterable_iterator(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE set_iterable_contains_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void set_mark_contents(VALUE UNUSED(self))
{

}

void init_set(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Set", &set_constructor, &set_destructor, "Iterable", CLS_SET);
    defineNativeMethod(vm, klass, &set_add, "add", 1, false);
    defineNativeMethod(vm, klass, &set_remove, "remove", 1, false);
}
