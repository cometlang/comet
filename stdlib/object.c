#include "comet.h"

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
    return OBJ_VAL(copyString("Object", 6));
}

VALUE obj_nil_q(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

void init_object(void)
{
    VALUE klass = defineNativeClass("Object", NULL, NULL, NULL);
    defineNativeMethod(klass, &obj_equals, "equals", false);
    defineNativeMethod(klass, &obj_hash, "hash", false);
    defineNativeMethod(klass, &obj_to_string, "to_string", false);
    defineNativeMethod(klass, &obj_nil_q, "nil?", false);
}
