#include "cometlib.h"
#include "comet.h"

static VALUE iterable_klass;
static VALUE iterator_klass;

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

void bootstrap_iterable(VM *vm)
{
    iterable_klass = bootstrapNativeClass(vm, "Iterable", NULL, NULL, CLS_ITERABLE);
    iterator_klass = bootstrapNativeClass(vm, "Iterator", NULL, NULL, CLS_ITERATOR);
}

void complete_iterable(VM *vm)
{
    completeNativeClassDefinition(vm, iterable_klass, NULL);
    defineNativeMethod(vm, iterable_klass, &iterable_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, iterable_klass, &iterable_empty_q, "empty?", 0, false);
    defineNativeMethod(vm, iterable_klass, &iterable_iterator, "iterator", 0, false);

    completeNativeClassDefinition(vm, iterator_klass, NULL);
    defineNativeMethod(vm, iterator_klass, &iterator_has_next_q, "has_next?", 0, false);
    defineNativeMethod(vm, iterator_klass, &iterator_get_next, "get_next", 0, false);
}
 