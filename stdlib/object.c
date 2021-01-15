#include <string.h>

#include "comet.h"
#include "cometlib.h"

VALUE instanceof(VALUE self, VALUE klass)
{
    if ((IS_INSTANCE(self) || IS_NATIVE_INSTANCE(self)) && IS_CLASS(klass))
    {
        ObjInstance *instance = AS_INSTANCE(self);
        if (instance->klass == AS_CLASS(klass))
        {
            return TRUE_VAL;
        }
        else if (instance->klass->super_ != NULL)
        {
            return instanceof(self, OBJ_VAL(instance->klass->super_));
        }
    }
    return FALSE_VAL;
}

VALUE obj_equals(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    Obj *rhs = AS_OBJ(arguments[0]);
    return BOOL_VAL(AS_OBJ(self) == rhs);
}

VALUE obj_hash(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    uint32_t hash = 2166136261u;
    uintptr_t address = (uintptr_t) AS_OBJ(self);

    for (size_t i = 0; i < sizeof(uintptr_t); i++)
    {
        hash ^= (address >> i) & 0xFF;
        hash *= 16777619;
    }

    return NUMBER_VAL(hash);
}

VALUE obj_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjInstance *instance = AS_INSTANCE(self);
    return copyString(vm, instance->klass->name, strlen(instance->klass->name));
}

VALUE obj_nil_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

void init_object(VM *vm, VALUE klass)
{
    defineNativeMethod(vm, klass, &obj_hash, "hash", false);
    defineNativeMethod(vm, klass, &obj_to_string, "to_string", false);
    defineNativeMethod(vm, klass, &obj_nil_q, "nil?", false);
    defineNativeOperator(vm, klass, &obj_equals, OPERATOR_EQUALS);
}
