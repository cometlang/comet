#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "cometlib.h"
#include "comet_stdlib.h"

static VALUE number_class;

VALUE number_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
#define TEMP_STRING_MAX_LEN 64
    char temp[TEMP_STRING_MAX_LEN];
    int length = snprintf(temp, TEMP_STRING_MAX_LEN, "%.17g", number_get_value(self));
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
    if (!IS_NUMBER(arg))
    {
        if (op == OPERATOR_EQUALS)
            return FALSE_VAL;

        throw_exception_native(vm, "ArgumentException", "argument must be a Number");
        return NIL_VAL;
    }

    switch (op)
    {
    case OPERATOR_MULTIPLICATION:
    {
        return create_number(vm, number_get_value(self) * number_get_value(arg));
    }
    case OPERATOR_PLUS:
    {
        return create_number(vm, number_get_value(self) + number_get_value(arg));
    }
    case OPERATOR_MINUS:
    {
        return create_number(vm, number_get_value(self) - number_get_value(arg));
    }
    case OPERATOR_DIVISION:
    {
        return create_number(vm, number_get_value(self) / number_get_value(arg));
    }
    case OPERATOR_MODULO:
    {
        return create_number(vm, ((int64_t)number_get_value(self) % (int64_t)number_get_value(arg)));
    }
    case OPERATOR_GREATER_THAN:
    {
        if (number_get_value(self) > number_get_value(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_LESS_THAN:
    {
        if (number_get_value(self) < number_get_value(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_GREATER_EQUAL:
    {
        if (number_get_value(self) >= number_get_value(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_LESS_EQUAL:
    {
        if (number_get_value(self) <= number_get_value(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_EQUALS:
    {
        if (number_get_value(self) == number_get_value(arg))
            return TRUE_VAL;
        return FALSE_VAL;
    }
    case OPERATOR_BITWISE_OR:
    {
        return create_number(vm, (double)((int64_t)number_get_value(self) | (int64_t)number_get_value(arg)));
    }
    case OPERATOR_BITWISE_AND:
    {
        return create_number(vm, (double)((int64_t)number_get_value(self) & (int64_t)number_get_value(arg)));
    }
    case OPERATOR_BITWISE_XOR:
    {
        return create_number(vm, (double)((int64_t)number_get_value(self) ^ (int64_t)number_get_value(arg)));
    }
    case OPERATOR_BITWISE_NEGATE:
    {
        return create_number(vm, (double)~((int64_t)number_get_value(self)));
    }
    case OPERATOR_BITSHIFT_LEFT:
    {
        return create_number(vm, (double)((int64_t)number_get_value(self) << (int64_t)number_get_value(arg)));
    }
    case OPERATOR_BITSHIFT_RIGHT:
    {
        return create_number(vm, (double)((int64_t)number_get_value(self) >> (int64_t)number_get_value(arg)));
    }
    default:
    {
        runtimeError(vm, "Operator '%s' is not defined for class 'Number'.", getOperatorString(op));
    }

    }
    return false;
}

VALUE number_pow(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    double result = pow(number_get_value(self), number_get_value(arguments[0]));
    return create_number(vm, result);
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
        return FALSE_VAL;
    return TRUE_VAL;
}

VALUE number_max(VM *vm, VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    double lhs = number_get_value(arguments[0]);
    double rhs = number_get_value(arguments[1]);
    return create_number(vm, fmax(lhs, rhs));
}

VALUE number_min(VM *vm, VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    double lhs = number_get_value(arguments[0]);
    double rhs = number_get_value(arguments[1]);
    return create_number(vm, fmin(lhs, rhs));
}

VALUE number_compare(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    return number_operator(vm, self, arguments, OPERATOR_EQUALS);
}

static unsigned int rand_seed;
VALUE number_random(VM *vm, VALUE UNUSED(klass), int arg_count, VALUE *arguments)
{
    if (arg_count == 1)
    {
        rand_seed = (int)number_get_value(arguments[0]);
    }
    double random;
#ifdef _WIN32
    random = (double)rand() / (double)RAND_MAX;
#else
    random = (double)rand_r(&rand_seed) / (double)RAND_MAX;
#endif
    return create_number(vm, random);
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
    rand_seed = time(NULL);
    srand(rand_seed);
}

void complete_number(VM *vm)
{
    completeNativeClassDefinition(vm, number_class, "Object");
    defineNativeMethod(vm, number_class, &number_to_string, "to_string", 0, false);
    defineNativeMethod(vm, number_class, &number_parse, "parse", 1, true);
    defineNativeMethod(vm, number_class, &number_random, "random", 0, true);
    defineNativeMethod(vm, number_class, &number_pow, "power", 1, false);
    defineNativeMethod(vm, number_class, &number_square_root, "square_root", 0, false);
    defineNativeMethod(vm, number_class, &number_ceiling, "ceiling", 0, false);
    defineNativeMethod(vm, number_class, &number_floor, "floor", 0, false);
    defineNativeMethod(vm, number_class, &number_even_p, "even?", 0, false);
    defineNativeMethod(vm, number_class, &number_abs, "absolute_value", 0, false);
    defineNativeMethod(vm, number_class, &number_max, "max", 2, true);
    defineNativeMethod(vm, number_class, &number_min, "min", 2, true);

    defineNativeOperator(vm, number_class, &number_compare, 1, OPERATOR_EQUALS);
}