#include "cometlib.h"
#include "comet_stdlib.h"

ObjNativeInstance boolean_true;
ObjNativeInstance boolean_false;

VALUE boolean_init(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE boolean_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString(vm, "Boolean, baby!", 14);
}

VALUE boolean_parse(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

static void init_instance(VM *vm, ObjNativeInstance *instance, ObjClass *klass, bool value)
{
    instance->instance.obj.type = OBJ_NATIVE_INSTANCE;
    instance->instance.obj.isMarked = false; // doesn't matter, it's static memory anyway
    instance->data = NULL;
    instance->instance.klass = klass;
    initTable(&instance->instance.fields);
    if (value)
        push(vm, copyString(vm, "true", 4));
    else
        push(vm, copyString(vm, "false", 5));
    addGlobal(vm, peek(vm, 0), OBJ_VAL(instance));
    pop(vm);
}

void init_boolean(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Boolean", NULL, NULL, NULL);
    defineNativeMethod(vm, klass, &boolean_init, "init", false);
    defineNativeMethod(vm, klass, &boolean_to_string, "to_string", false);
    defineNativeMethod(vm, klass, &boolean_parse, "parse", true);
    init_instance(vm, &boolean_true, AS_CLASS(klass), true);
    init_instance(vm, &boolean_false, AS_CLASS(klass), false);
}