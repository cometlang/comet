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

VALUE iterable_max(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    VALUE iterator_func_name = copyString(vm, "iterator", strlen("iterator"));
    push(vm, iterator_func_name);
    VALUE has_next_name = copyString(vm, "has_next?", strlen("has_next?"));
    push(vm, has_next_name);
    VALUE get_next_name = copyString(vm, "get_next", strlen("get_next"));
    push(vm, get_next_name);
    VALUE iterator = call_function(self, iterator_func_name, 0, NULL);
    push(vm, iterator);
    VALUE has_next = call_function(iterator, has_next_name, 0, NULL);
    push(vm, has_next);
    VALUE current = NIL_VAL;
    while (has_next == TRUE_VAL) {
        VALUE item = call_function(iterator, get_next_name, 0, NULL);
        push(vm, item);
        if (current == NIL_VAL) {
            current = item;
        } else {
            VALUE val = item;
            if (arg_count == 1) {
                val = call_function(item, arguments[0], 1, &item);
                pop(vm);
                push(vm, val);
            }
            VALUE greater_than_func = AS_INSTANCE(val)->klass->operators[OPERATOR_GREATER_THAN];
            VALUE result = call_function(val, greater_than_func, 1, &current);
            if (result == TRUE_VAL) {
                current = val;
            }
        }
        popMany(vm, 2);
        has_next = call_function(iterator, has_next_name, 0, NULL);
        push(vm, has_next);
    }
    popMany(vm, 5);
    return current;
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

    completeNativeClassDefinition(vm, iterator_klass, NULL);
    defineNativeMethod(vm, iterator_klass, &iterator_has_next_q, "has_next?", 0, false);
    defineNativeMethod(vm, iterator_klass, &iterator_get_next, "get_next", 0, false);
}
 