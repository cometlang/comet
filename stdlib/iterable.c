#include "cometlib.h"
#include "comet.h"

VALUE iterable_contains_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterable_empty_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterable_iterator(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterator_has_next_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterator_get_next(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

void init_iterable(VM *vm)
{
    VALUE iterable_klass = defineNativeClass(vm, "Iterable", NULL, NULL, NULL);
    defineNativeMethod(vm, iterable_klass, &iterable_contains_q, "contains?", false);
    defineNativeMethod(vm, iterable_klass, &iterable_empty_q, "empty?", false);
    defineNativeMethod(vm, iterable_klass, &iterable_iterator, "iterator", false);

    VALUE iterator_klass = defineNativeClass(vm, "Iterator", NULL, NULL, NULL);
    defineNativeMethod(vm, iterator_klass, &iterator_has_next_q, "has_next?", false);
    defineNativeMethod(vm, iterator_klass, &iterator_get_next, "get_next", false);
}
 