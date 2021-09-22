#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "cometlib.h"
#include "comet_stdlib.h"

static VALUE number_class;

void *number_constructor()
{
    NumberData *data = (NumberData *) ALLOCATE(NumberData, 1);
    data->num = 0;
    return data;
}

void number_destructor(void *data)
{
    FREE(NumberData, data);
}

VALUE number_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *data = GET_NATIVE_INSTANCE_DATA(NumberData, self);
#define TEMP_STRING_MAX_LEN 64
    char temp[TEMP_STRING_MAX_LEN];
    int length = snprintf(temp, TEMP_STRING_MAX_LEN, "%.17g", data->num);
    return copyString(vm, temp, length);
#undef TEMP_STRING_MAX_LEN
}

VALUE number_parse(VM *vm, VALUE UNUSED(klass), int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        VALUE arg = arguments[0];
        if (instanceof(arg, number_class) == TRUE_VAL)
        {
            return arg;
        }

        const char *string = string_get_cstr(arg);
        char *failed;
        double value = strtod(string, &failed);
        if (failed != string)
            return create_number(vm, value);
    }
    return NIL_VAL;
}

VALUE number_square_root(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *data = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    double result = sqrt(data->num);
    return create_number(vm, result);
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

VALUE number_operator_greater_than(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    if (lhs->num > rhs->num)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE number_operator_greater_equal(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    if (lhs->num >= rhs->num)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE number_operator_less_than(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    if (lhs->num < rhs->num)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE number_operator_less_equal(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    if (lhs->num <= rhs->num)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE number_operator_equals(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData *lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData *rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    if (lhs->num == rhs->num)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE number_operator_bitwise_or(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData* lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData* rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, (double) ((int64_t)lhs->num | (int64_t)rhs->num));
}

VALUE number_operator_bitwise_and(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData* lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData* rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, (double)((int64_t)lhs->num & (int64_t)rhs->num));
}

VALUE number_operator_bitwise_xor(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    NumberData* lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData* rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, (double)((int64_t)lhs->num ^ (int64_t)rhs->num));
}

VALUE number_operator_bitwise_negate(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData* lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    return create_number(vm, (double)(~(int64_t)lhs->num));
}

VALUE number_operator_bitshift_left(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData* lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData* rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, (double)((int64_t)lhs->num << (int64_t)rhs->num));
}

VALUE number_operator_bitshift_right(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    NumberData* lhs = GET_NATIVE_INSTANCE_DATA(NumberData, self);
    NumberData* rhs = GET_NATIVE_INSTANCE_DATA(NumberData, arguments[0]);
    return create_number(vm, (double)((int64_t)lhs->num >> (int64_t)rhs->num));
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
        NumberData *data = GET_NATIVE_INSTANCE_DATA(NumberData, self);
        if (data != NULL)
            return data->num;
    }
    return NAN;
}

bool is_a_number(VALUE instance)
{
    if (instanceof(instance, number_class) == TRUE_VAL)
        return true;
    return false;
}

void bootstrap_number(VM *vm)
{
    number_class = bootstrapNativeClass(vm, "Number", &number_constructor, &number_destructor, CLS_NUMBER, false);
}

void complete_number(VM *vm)
{
    completeNativeClassDefinition(vm, number_class, NULL);
    defineNativeMethod(vm, number_class, &number_to_string, "to_string", 0, false);
    defineNativeMethod(vm, number_class, &number_parse, "parse", 1, true);
    defineNativeMethod(vm, number_class, &number_square_root, "square_root", 0, false);
    defineNativeOperator(vm, number_class, &number_operator_plus, 1, OPERATOR_PLUS);
    defineNativeOperator(vm, number_class, &number_operator_minus, 1, OPERATOR_MINUS);
    defineNativeOperator(vm, number_class, &number_operator_divide, 1, OPERATOR_DIVISION);
    defineNativeOperator(vm, number_class, &number_operator_multiply, 1, OPERATOR_MULTIPLICATION);
    defineNativeOperator(vm, number_class, &number_operator_modulo, 1, OPERATOR_MODULO);

    defineNativeOperator(vm, number_class, &number_operator_bitwise_or, 1, OPERATOR_BITWISE_OR);
    defineNativeOperator(vm, number_class, &number_operator_bitwise_and, 1, OPERATOR_BITWISE_AND);
    defineNativeOperator(vm, number_class, &number_operator_bitwise_xor, 1, OPERATOR_BITWISE_XOR);
    defineNativeOperator(vm, number_class, &number_operator_bitshift_left, 1, OPERATOR_BITSHIFT_LEFT);
    defineNativeOperator(vm, number_class, &number_operator_bitshift_right, 1, OPERATOR_BITSHIFT_RIGHT);
    defineNativeOperator(vm, number_class, &number_operator_bitwise_negate, 0, OPERATOR_BITWISE_NEGATE);

    // Ideally I should be able to compress these five down to one eventually
    defineNativeOperator(vm, number_class, &number_operator_greater_than, 1, OPERATOR_GREATER_THAN);
    defineNativeOperator(vm, number_class, &number_operator_greater_equal, 1, OPERATOR_GREATER_EQUAL);
    defineNativeOperator(vm, number_class, &number_operator_less_than, 1, OPERATOR_LESS_THAN);
    defineNativeOperator(vm, number_class, &number_operator_less_equal, 1, OPERATOR_LESS_EQUAL);
    defineNativeOperator(vm, number_class, &number_operator_equals, 1, OPERATOR_EQUALS);
}