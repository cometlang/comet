#include <stdint.h>
#include "comet.h"

VALUE nil_nil_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return TRUE_VAL;
}

VALUE nil_to_string(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString("nil", 3);
}

VALUE nil_iterable_empty_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return TRUE_VAL;
}

VALUE nil_iterable_iterator(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE nil_iterable_contains_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

void init_nil(void)
{
    VALUE klass = defineNativeClass("nil", NULL, NULL, "Object");
    defineNativeMethod(klass, nil_nil_q, "nil?", true);
    defineNativeMethod(klass, nil_to_string, "to_string", true);
    defineNativeMethod(klass, nil_iterable_contains_q, "contains?", true);
}
