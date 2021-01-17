#include "cometlib.h"

typedef union {
  uint64_t bits64;
  uint32_t bits32[2];
  double num;
} NumberData;

void *number_constructor()
{
    NumberData *data = ALLOCATE(NumberData, 1);
    data->bits64 = 0;
    return data;
}

void number_destructor(void *data)
{
    FREE(NumberData, data);
}

VALUE number_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return copyString(vm, "number", 6);
}

VALUE number_operator_plus(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE number_operator_minus(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE number_operator_divide(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE number_operator_multiply(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_number(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Number", &number_constructor, &number_destructor, NULL);
    defineNativeMethod(vm, klass, &number_to_string, "to_string", false);
    defineNativeOperator(vm, klass, &number_operator_plus, OPERATOR_PLUS);
    defineNativeOperator(vm, klass, &number_operator_minus, OPERATOR_MINUS);
    defineNativeOperator(vm, klass, &number_operator_divide, OPERATOR_DIVISION);
    defineNativeOperator(vm, klass, &number_operator_multiply, OPERATOR_MULTIPLICATION);
}