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
    else if (IS_NUMBER(self) && IS_NATIVE_CLASS(klass)) {
        if (AS_CLASS(klass)->classType == CLS_NUMBER) {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
}

VALUE obj_compare_to(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    if (self == arguments[0])
        return create_number(vm, 0);
    return NIL_VAL;
}

VALUE obj_equals(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    VALUE result = obj_compare_to(vm, self, arg_count, arguments);
    if (result == NIL_VAL)
        return FALSE_VAL;
    return TRUE_VAL;
}

VALUE obj_hash(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    uint32_t hash = 2166136261u;
    uintptr_t address = (uintptr_t) AS_OBJ(self);

    for (size_t i = 0; i < sizeof(uintptr_t); i++)
    {
        hash ^= (address >> i) & 0xFF;
        hash *= 16777619;
    }

    return create_number(vm, (double) hash);
}

// A placeholder such that anyone can call "super.init()"
VALUE obj_init(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE obj_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    if (IS_NUMBER(self))
    {
        return copyString(vm, "Number instance", 15);
    }
    ObjInstance *instance = AS_INSTANCE(self);
#ifdef WIN32
#define string_len 256
#else
    size_t string_len = strlen(instance->klass->name) + strlen(" instance") + 1;
#endif
    char string[string_len];
    snprintf(string, string_len, "%s instance", instance->klass->name);
    return copyString(vm, string, string_len);
}

VALUE cls_to_string(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    if (IS_NUMBER(klass))
    {
        return copyString(vm, "Number class", 12);
    }
#ifdef WIN32
#define string_len 256
#else
    size_t string_len = strlen(AS_CLASS(klass)->name) + strlen(" class") + 1;
#endif
    char string[string_len];
    snprintf(string, string_len, "%s class", AS_CLASS(klass)->name);
    return copyString(vm, string, string_len);
}

VALUE obj_nil_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

VALUE obj_methods(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE list = list_create(vm);
    push(vm, list);
    ObjClass *klass = AS_INSTANCE(self)->klass;
    tableGetKeys(&klass->methods, vm, list);
    return pop(vm);
}

VALUE obj_fields(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE list = list_create(vm);
    push(vm, list);
    ObjInstance *obj = AS_INSTANCE(self);
    tableGetKeys(&obj->fields, vm, list);
    return pop(vm);
}

void init_object(VM *vm, VALUE klass)
{
    defineNativeMethod(vm, klass, &obj_hash, "hash", 0, false);
    defineNativeMethod(vm, klass, &obj_init, "init", 0, false);
    defineNativeMethod(vm, klass, &obj_to_string, "to_string", 0, false);
    defineNativeMethod(vm, klass, &obj_to_string, "class_name", 0, false);
    defineNativeMethod(vm, klass, &cls_to_string, "to_string", 0, true);
    defineNativeMethod(vm, klass, &obj_hash, "hash", 0, true);
    defineNativeMethod(vm, klass, &obj_nil_q, "nil?", 0, false);
    defineNativeMethod(vm, klass, &obj_compare_to, "compare_to", 1, false);
    defineNativeMethod(vm, klass, &obj_methods, "methods", 0, false);
    defineNativeMethod(vm, klass, &obj_fields, "fields", 0, false);
    defineNativeOperator(vm, klass, &obj_equals, 1, OPERATOR_EQUALS);
}
