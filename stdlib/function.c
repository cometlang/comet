#include "comet.h"

VALUE function_name(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjFunction *func = AS_CLOSURE(self)->function;
    return func->name;
}

VALUE function_attributes(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjFunction *func = AS_CLOSURE(self)->function;
    VALUE list = list_create(vm);
    push(vm, list);
    list_add(vm, list, func->attributeCount, func->attributes);
    return pop(vm);
}

void init_function(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Function", NULL, NULL, NULL, NULL, CLS_FUNCTION, 0, true);
    defineNativeMethod(vm, klass, &function_attributes, "attributes", 0, false);
    defineNativeMethod(vm, klass, &function_name, "name", 0, false);
}