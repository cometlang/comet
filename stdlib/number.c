#include <stdio.h>
#include "cometlib.h"
#include "comet_stdlib.h"

typedef union {
  uint64_t bits64;
  uint32_t bits32[2];
  double num;
} NumberData;

static VALUE number_class;

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

VALUE number_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *data = GET_NATIVE_INSTANCE_DATA(NumberData, self);
#define TEMP_STRING_MAX_LEN 64
    char temp[TEMP_STRING_MAX_LEN];
    int length = snprintf(temp, TEMP_STRING_MAX_LEN, "%ld", (int64_t) data->num);
    return copyString(vm, temp, length);
#undef TEMP_STRING_MAX_LEN
}

VALUE number_operator_plus(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(number_class)));
    NumberData *result_data = GET_NATIVE_INSTANCE_DATA(NumberData, result);
    result_data->num = lhs->num + rhs->num;
    return result;
}

VALUE number_operator_minus(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(number_class)));
    NumberData *result_data = GET_NATIVE_INSTANCE_DATA(NumberData, result);
    result_data->num = lhs->num - rhs->num;
    return result;
}

VALUE number_operator_divide(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(number_class)));
    NumberData *result_data = GET_NATIVE_INSTANCE_DATA(NumberData, result);
    result_data->num = lhs->num / rhs->num;
    return result;
}

VALUE number_operator_multiply(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(number_class)));
    NumberData *result_data = GET_NATIVE_INSTANCE_DATA(NumberData, result);
    result_data->num = lhs->num * rhs->num;
    return result;
}

void init_number(VM *vm)
{
    number_class = defineNativeClass(vm, "Number", &number_constructor, &number_destructor, NULL);
    defineNativeMethod(vm, number_class, &number_to_string, "to_string", false);
    defineNativeOperator(vm, number_class, &number_operator_plus, OPERATOR_PLUS);
    defineNativeOperator(vm, number_class, &number_operator_minus, OPERATOR_MINUS);
    defineNativeOperator(vm, number_class, &number_operator_divide, OPERATOR_DIVISION);
    defineNativeOperator(vm, number_class, &number_operator_multiply, OPERATOR_MULTIPLICATION);
}