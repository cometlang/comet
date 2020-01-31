#include <stdint.h>
#include "comet.h"

VALUE nil_nil_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return BOOL_VAL(true);
}

VALUE nil_to_string(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return OBJ_VAL(copyString("nil", 3));
}

VALUE nil_iterable_empty_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return BOOL_VAL(true);
}

VALUE nil_iterable_iterator(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE nil_iterable_contains_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return BOOL_VAL(false);
}


// create type
// extend object
// override method