#include <stdint.h>
#include "comet.h"

ObjNativeInstance nil_instance;

VALUE nil_nil_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return TRUE_VAL;
}

VALUE nil_to_string(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString(vm, "nil", 3);
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

void init_nil(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Nil", NULL, NULL, "Object");
    defineNativeMethod(vm, klass, nil_nil_q, "nil?", false);
    defineNativeMethod(vm, klass, nil_to_string, "to_string", false);
    defineNativeMethod(vm, klass, nil_iterable_contains_q, "contains?", false);
    nil_instance.instance.obj.type = OBJ_NATIVE_INSTANCE;
    nil_instance.instance.obj.isMarked = false; // doesn't matter, it's static memory anyway
    nil_instance.data = NULL;
    nil_instance.instance.klass = AS_CLASS(klass);
    initTable(&nil_instance.instance.fields);
    push(vm, copyString(vm, "nil", 3));
    addGlobal(vm, peek(vm, 0), NIL_VAL);
    pop(vm);
}
