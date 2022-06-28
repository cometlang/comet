#include <stdio.h>

#include "cometlib.h"
#include "comet_stdlib.h"

typedef struct {
    ObjNativeInstance obj;
    bool value;
} BooleanData_t;

static BooleanData_t _true;
static BooleanData_t _false;

ObjNativeInstance *boolean_true = (ObjNativeInstance *) &_true;
ObjNativeInstance *boolean_false = (ObjNativeInstance *) &_false;

static VALUE bool_class;

VALUE boolean_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    BooleanData_t *data = GET_NATIVE_INSTANCE_DATA(BooleanData_t, self);
    if (data->value)
        return copyString(vm, "true", 4);
    return copyString(vm, "false", 5);
}

VALUE boolean_parse(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

static void init_instance(VM *vm, BooleanData_t *instance, ObjClass *klass, bool value)
{
    instance->obj.instance.obj.type = OBJ_NATIVE_INSTANCE;
    instance->obj.instance.obj.isMarked = false; // doesn't matter, it's static memory anyway
    instance->obj.instance.klass = klass;
    instance->value = value;
    initTable(&instance->obj.instance.fields);
    if (value)
        push(vm, copyString(vm, "true", 4));
    else
        push(vm, copyString(vm, "false", 5));
    addGlobal(peek(vm, 0), OBJ_VAL(instance));
    pop(vm);
}

bool bool_get_value(VALUE value)
{
    DEBUG_ASSERT(instanceof(value, bool_class));
    BooleanData_t *data = GET_NATIVE_INSTANCE_DATA(BooleanData_t, value);
    return (bool)data->value;
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
    bool_class = defineNativeClass(vm, "Boolean", NULL, NULL, NULL, NULL, CLS_BOOLEAN, sizeof(BooleanData_t), true);
    defineNativeMethod(vm, bool_class, &boolean_to_string, "to_string", 0, false);
    defineNativeMethod(vm, bool_class, &boolean_parse, "parse", 1, true);
    init_instance(vm, &_true, AS_CLASS(bool_class), true);
    init_instance(vm, &_false, AS_CLASS(bool_class), false);
}