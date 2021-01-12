#include "comet.h"

VALUE instanceof(VALUE self, VALUE klass)
{
    Obj *lhs = AS_OBJ(self);
    if ((lhs->type == OBJ_INSTANCE || lhs->type == OBJ_NATIVE_INSTANCE) && IS_CLASS(klass))
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

VALUE obj_equals(VALUE UNUSED(self), int UNUSED(arg_count), VALUE *arguments)
{
    Obj *rhs = AS_OBJ(arguments[0]);
    return BOOL_VAL(AS_OBJ(self) == rhs);
}

VALUE obj_hash(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
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

VALUE obj_to_string(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString("Object", 6);
}

VALUE obj_nil_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

void init_object(void)
{
    VALUE klass = defineNativeClass("Object", NULL, NULL, NULL);
    defineNativeMethod(klass, &obj_hash, "hash", false);
    defineNativeMethod(klass, &obj_to_string, "to_string", false);
    defineNativeMethod(klass, &obj_nil_q, "nil?", false);
    defineNativeOperator(klass, &obj_equals, OPERATOR_EQUALS);
}
