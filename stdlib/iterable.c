#include "cometlib.h"
#include "comet.h"
#include <string.h>
#include <stdio.h>

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

VALUE iterable_count(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return create_number(vm, 0);
}

VALUE iterator_has_next_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterator_get_next(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE iterable_compare(VM* vm, VALUE self, int arg_count, VALUE* arguments, OPERATOR op)
{
    VALUE iterator_func_name = copyString(vm, "iterator", strlen("iterator"));
    push(vm, iterator_func_name);
    VALUE has_next_name = copyString(vm, "has_next?", strlen("has_next?"));
    push(vm, has_next_name);
    VALUE get_next_name = copyString(vm, "get_next", strlen("get_next"));
    push(vm, get_next_name);
    call_function(vm, self, iterator_func_name, 0, NULL);
    VALUE iterator = peek(vm, 0);
    call_function(vm, iterator, has_next_name, 0, NULL);
    VALUE has_next = peek(vm, 0);
    VALUE current = NIL_VAL;
    while (has_next == TRUE_VAL) {
        call_function(vm, iterator, get_next_name, 0, NULL);
        VALUE item = peek(vm, 0);
        if (current == NIL_VAL) {
            current = item;
        }
        else {
            VALUE val = item;
            if (arg_count == 1) {
                call_function(vm, item, arguments[0], 1, &item);
                swapTop(vm);
                pop(vm);
                val = peek(vm, 0);
            }
            VALUE compare_func = AS_INSTANCE(val)->klass->operators[op];
            call_function(vm, val, compare_func, 1, &current);
            VALUE result = peek(vm, 0);
            if (result == TRUE_VAL) {
                current = val;
            }
        }
        popMany(vm, 2);
        call_function(vm, iterator, has_next_name, 0, NULL);
        has_next = peek(vm, 0);
    }
    popMany(vm, 5);
    return current;
}

VALUE iterable_max(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    return iterable_compare(vm, self, arg_count, arguments, OPERATOR_GREATER_THAN);
}

VALUE iterable_min(VM* vm, VALUE self, int arg_count, VALUE* arguments)
{
    return iterable_compare(vm, self, arg_count, arguments, OPERATOR_LESS_THAN);
}

void bootstrap_iterable(VM *vm)
{
    iterable_klass = bootstrapNativeClass(vm, "Iterable", NULL, NULL, CLS_ITERABLE, 0, false);
    iterator_klass = bootstrapNativeClass(vm, "Iterator", NULL, NULL, CLS_ITERATOR, 0, false);
}

void complete_iterable(VM *vm)
{
    completeNativeClassDefinition(vm, iterable_klass, NULL);
    defineNativeMethod(vm, iterable_klass, &iterable_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, iterable_klass, &iterable_empty_q, "empty?", 0, false);
    defineNativeMethod(vm, iterable_klass, &iterable_iterator, "iterator", 0, false);
    defineNativeMethod(vm, iterable_klass, &iterable_count, "count", 0, false);
    defineNativeMethod(vm, iterable_klass, &iterable_max, "max", 0, false);
    defineNativeMethod(vm, iterable_klass, &iterable_min, "min", 0, false);

    completeNativeClassDefinition(vm, iterator_klass, NULL);
    defineNativeMethod(vm, iterator_klass, &iterator_has_next_q, "has_next?", 0, false);
    defineNativeMethod(vm, iterator_klass, &iterator_get_next, "get_next", 0, false);
}
 