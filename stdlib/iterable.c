#include "cometlib.h"
#include "comet.h"

VALUE iterable_contains_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterable_empty_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterable_iterator(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterator_has_next_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterator_get_next(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

void init_iterable(VM UNUSED(*vm))
{
    VALUE iterable_klass = defineNativeClass("Iterable", NULL, NULL, NULL);
    defineNativeMethod(iterable_klass, &iterable_contains_q, "contains?", false);
    defineNativeMethod(iterable_klass, &iterable_empty_q, "empty?", false);
    defineNativeMethod(iterable_klass, &iterable_iterator, "iterator", false);

    VALUE iterator_klass = defineNativeClass("Iterator", NULL, NULL, NULL);
    defineNativeMethod(iterator_klass, &iterator_has_next_q, "has_next?", false);
    defineNativeMethod(iterator_klass, &iterator_get_next, "get_next", false);
}
 