#include <stdint.h>
#include "comet.h"

ObjNativeInstance nil_instance;
VALUE nil_iterator_class;

VALUE nil_iterator_has_next_p(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE nil_iterator_get_next(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE nil_nil_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return TRUE_VAL;
}

VALUE nil_to_string(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString(vm, "", 0);
}

VALUE nil_iterable_empty_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return TRUE_VAL;
}

VALUE nil_iterable_iterator(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE nil_iterable_contains_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE nil_iterable_count(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return create_number(vm, 0);
}

void init_nil(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Nil", NULL, NULL, "Iterable", CLS_NIL);
    defineNativeMethod(vm, klass, &nil_nil_q, "nil?", 0, false);
    defineNativeMethod(vm, klass, &nil_to_string, "to_string", 0, false);
    defineNativeMethod(vm, klass, &nil_iterable_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, klass, &nil_iterable_iterator, "iterator", 0, false);
    defineNativeMethod(vm, klass, &nil_iterable_count, "count", 0, false);
    nil_instance.instance.obj.type = OBJ_NATIVE_INSTANCE;
    nil_instance.instance.obj.isMarked = false; // doesn't matter, it's static memory anyway
    nil_instance.data = NULL;
    nil_instance.instance.klass = AS_CLASS(klass);
    initTable(&nil_instance.instance.fields);
    push(vm, copyString(vm, "nil", 3));
    addGlobal(peek(vm, 0), NIL_VAL);
    pop(vm);

    nil_iterator_class = defineNativeClass(vm, "NilIterator", NULL, NULL, "Iterator", CLS_ITERATOR);
    defineNativeMethod(vm, nil_iterator_class, &nil_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, nil_iterator_class, &nil_iterator_get_next, "get_next", 0, false);
}
