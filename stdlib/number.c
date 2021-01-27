#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
    NumberData *data = (NumberData *) malloc(sizeof(NumberData));
    data->bits64 = 0;
    return data;
}

void number_destructor(void *data)
{
    free(data);
}

VALUE number_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *data = GET_NATIVE_INSTANCE_DATA(NumberData, self);
#define TEMP_STRING_MAX_LEN 64
    char temp[TEMP_STRING_MAX_LEN];
    int length = snprintf(temp, TEMP_STRING_MAX_LEN, "%.17g", data->num);
    return copyString(vm, temp, length);
#undef TEMP_STRING_MAX_LEN
}

VALUE number_parse(VM *vm, VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return create_number(vm, 0);
}

VALUE number_operator_plus(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, lhs->num + rhs->num);
}

VALUE number_operator_minus(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, lhs->num - rhs->num);
}

VALUE number_operator_divide(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, lhs->num / rhs->num);
}

VALUE number_operator_multiply(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, lhs->num * rhs->num);
}

VALUE number_operator_modulo(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, (int64_t) lhs->num % (int64_t) rhs->num);
}

VALUE create_number(VM *vm, double number)
{
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(number_class)));
    NumberData *result_data = GET_NATIVE_INSTANCE_DATA(NumberData, result);
    result_data->num = number;
    return result;
}

double number_get_value(VALUE self)
{
    if (instanceof(self, number_class) == TRUE_VAL)
    {
        return GET_NATIVE_INSTANCE_DATA(NumberData, self)->num;
    }
    return NAN;
}

void bootstrap_number(VM *vm)
{
    number_class = bootstrapNativeClass(vm, "Number", &number_constructor, &number_destructor);
}

void complete_number(VM *vm)
{
    completeNativeClassDefinition(vm, number_class, NULL);
    defineNativeMethod(vm, number_class, &number_to_string, "to_string", false);
    defineNativeMethod(vm, number_class, &number_parse, "parse", true);
    defineNativeOperator(vm, number_class, &number_operator_plus, OPERATOR_PLUS);
    defineNativeOperator(vm, number_class, &number_operator_minus, OPERATOR_MINUS);
    defineNativeOperator(vm, number_class, &number_operator_divide, OPERATOR_DIVISION);
    defineNativeOperator(vm, number_class, &number_operator_multiply, OPERATOR_MULTIPLICATION);
    defineNativeOperator(vm, number_class, &number_operator_modulo, OPERATOR_MODULO);
}