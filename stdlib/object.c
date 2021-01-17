#include <string.h>
#include <stdio.h>

#include "comet.h"
#include "cometlib.h"

VALUE instanceof(VALUE self, VALUE klass)
{
    if ((IS_INSTANCE(self) || IS_NATIVE_INSTANCE(self)) &&
        (IS_CLASS(klass) || IS_NATIVE_CLASS(klass)))
    {
        ObjInstance *instance = AS_INSTANCE(self);
        if (instance->klass == AS_CLASS(klass))
        {
            return TRUE_VAL;
        }
        ObjClass *current_klass = instance->klass->super_;
        while (current_klass != NULL)
        {
            if (current_klass == AS_CLASS(klass))
                return TRUE_VAL;
            current_klass = current_klass->super_;
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
    size_t string_len = strlen(instance->klass->name) + strlen(" instance") + 1;
    char string[string_len];
    snprintf(string, string_len, "%s instance", instance->klass->name);
    return copyString(vm, string, string_len);
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
