#include "cometlib.h"
#include "comet_stdlib.h"

ObjNativeInstance boolean_true;
ObjNativeInstance boolean_false;
static VALUE bool_class;

VALUE boolean_init(VM UNUSED(*vm), VALUE self, int arg_count, VALUE *arguments)
{
    if (arg_count == 1 && !bool_is_falsey(arguments[0]))
    {
        AS_NATIVE_INSTANCE(self)->data = (void *) true;
    }
    else
    {
        AS_NATIVE_INSTANCE(self)->data = (void *) false;
    }

    return NIL_VAL;
}

VALUE boolean_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    bool value = (bool) AS_NATIVE_INSTANCE(self)->data;
    if (value)
        return copyString(vm, "true", 4);
    return copyString(vm, "false", 5);
}

VALUE boolean_parse(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

static void init_instance(VM *vm, ObjNativeInstance *instance, ObjClass *klass, bool value)
{
    instance->instance.obj.type = OBJ_NATIVE_INSTANCE;
    instance->instance.obj.isMarked = false; // doesn't matter, it's static memory anyway
    instance->data = (void *) value;
    instance->instance.klass = klass;
    initTable(&instance->instance.fields);
    if (value)
        push(vm, copyString(vm, "true", 4));
    else
        push(vm, copyString(vm, "false", 5));
    addGlobal(vm, peek(vm, 0), OBJ_VAL(instance));
    pop(vm);
}

bool bool_get_value(VALUE value)
{
    DEBUG_ASSERT(instanceof(value, bool_class));
    return (bool) AS_NATIVE_INSTANCE(value)->data;
}

bool bool_is_falsey(VALUE value)
{
    if (IS_NIL(value))
        return true;
    if (instanceof(value, bool_class) && !bool_get_value(value))
        return true;

    return false;
}

void init_boolean(VM *vm)
{
    bool_class = defineNativeClass(vm, "Boolean", NULL, NULL, NULL);
    defineNativeMethod(vm, bool_class, &boolean_init, "init", false);
    defineNativeMethod(vm, bool_class, &boolean_to_string, "to_string", false);
    defineNativeMethod(vm, bool_class, &boolean_parse, "parse", true);
    init_instance(vm, &boolean_true, AS_CLASS(bool_class), true);
    init_instance(vm, &boolean_false, AS_CLASS(bool_class), false);
}