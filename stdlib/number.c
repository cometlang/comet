#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "cometlib.h"
#include "comet_stdlib.h"

static VALUE number_class;

VALUE number_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
#define TEMP_STRING_MAX_LEN 64
    char temp[TEMP_STRING_MAX_LEN];
    int length = snprintf(temp, TEMP_STRING_MAX_LEN, "%.17g", AS_NUMBER(self));
    return copyString(vm, temp, length);
#undef TEMP_STRING_MAX_LEN
}

VALUE number_parse(VM *vm, VALUE UNUSED(klass), int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        VALUE arg = arguments[0];
        if (IS_NUMBER(arg))
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

/**
* This is absolutely a concession to speed.  Basically, polymorphism won't
* work for a Number, because I'm not performing a lookup.  And I don't have
* an object instance to get the overidden function table.
**/
VALUE number_operator(VM* vm, VALUE self, VALUE* arguments, OPERATOR op)
{
    VALUE arg = arguments == NULL ? NIL_VAL : arguments[0];
    switch (op)
    {
    case OPERATOR_MULTIPLICATION:
    {
        return create_number(vm, AS_NUMBER(self) * AS_NUMBER(arg));
    }
    case OPERATOR_PLUS:
    {
        return create_number(vm, AS_NUMBER(self) + AS_NUMBER(arg));
    }
    case OPERATOR_MINUS:
    {
        return create_number(vm, AS_NUMBER(self) - AS_NUMBER(arg));
    }
    case OPERATOR_DIVISION:
    {
        return create_number(vm, AS_NUMBER(self) / AS_NUMBER(arg));
    }
    case OPERATOR_MODULO:
    {
        return create_number(vm, ((int64_t)AS_NUMBER(self) % (int64_t)AS_NUMBER(arg)));
    }
    case OPERATOR_GREATER_THAN:
    {
        if (AS_NUMBER(self) > AS_NUMBER(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_LESS_THAN:
    {
        if (AS_NUMBER(self) < AS_NUMBER(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_GREATER_EQUAL:
    {
        if (AS_NUMBER(self) >= AS_NUMBER(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_LESS_EQUAL:
    {
        if (AS_NUMBER(self) <= AS_NUMBER(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_EQUALS:
    {
        if (AS_NUMBER(self) == AS_NUMBER(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_BITWISE_OR:
    {
        return create_number(vm, (double)((int64_t)AS_NUMBER(self) | (int64_t)AS_NUMBER(arg)));
    }
    case OPERATOR_BITWISE_AND:
    {
        return create_number(vm, (double)((int64_t)AS_NUMBER(self) & (int64_t)AS_NUMBER(arg)));
    }
    case OPERATOR_BITWISE_XOR:
    {
        return create_number(vm, (double)((int64_t)AS_NUMBER(self) ^ (int64_t)AS_NUMBER(arg)));
    }
    case OPERATOR_BITWISE_NEGATE:
    {
        return create_number(vm, (double)~((int64_t)AS_NUMBER(self)));
    }
    case OPERATOR_BITSHIFT_LEFT:
    {
        return create_number(vm, (double)((int64_t)AS_NUMBER(self) << (int64_t)AS_NUMBER(arg)));
    }
    case OPERATOR_BITSHIFT_RIGHT:
    {
        return create_number(vm, (double)((int64_t)AS_NUMBER(self) >> (int64_t)AS_NUMBER(arg)));
    }
    default:
    {
        runtimeError(vm, "Undefined operator for Number class: %s", getOperatorString(op));
    }

    }
    return false;
}


VALUE number_square_root(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    double result = sqrt(number_get_value(self));
    return create_number(vm, result);
}

VALUE number_abs(VM* vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    double result = fabs(number_get_value(self));
    return create_number(vm, result);
}

VALUE number_ceiling(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    double result = ceil(number_get_value(self));
    return create_number(vm, result);
}

VALUE number_floor(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    double result = floor(number_get_value(self));
    return create_number(vm, result);
}

VALUE number_even_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    if (((int64_t)number_get_value(self)) & 1)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE create_number(VM UNUSED(*vm), double number)
{
    return NUMBER_VAL(number);
}

double number_get_value(VALUE self)
{
    if (IS_NUMBER(self))
    {
        return AS_NUMBER(self);
    }
    return NAN;
}

void bootstrap_number(VM *vm)
{
    number_class = bootstrapNativeClass(vm, "Number", NULL, NULL, CLS_NUMBER, 0, true);
}

void complete_number(VM *vm)
{
    completeNativeClassDefinition(vm, number_class, NULL);
    defineNativeMethod(vm, number_class, &number_to_string, "to_string", 0, false);
    defineNativeMethod(vm, number_class, &number_parse, "parse", 1, true);
    defineNativeMethod(vm, number_class, &number_square_root, "square_root", 0, false);
    defineNativeMethod(vm, number_class, &number_ceiling, "ceiling", 0, false);
    defineNativeMethod(vm, number_class, &number_floor, "floor", 0, false);
    defineNativeMethod(vm, number_class, &number_even_p, "even?", 0, false);
    defineNativeMethod(vm, number_class, &number_abs, "absolute_value", 0, false);
}