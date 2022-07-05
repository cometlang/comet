#include <chrono>
#include <date/date.h>
#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"
#include "datetime.hpp"

static VALUE duration_class;

typedef struct {
    ObjInstance obj;
    std::chrono::nanoseconds duration;
} DurationData;

std::chrono::nanoseconds duration_get_value(VALUE duration)
{
    DurationData *data = GET_NATIVE_INSTANCE_DATA(DurationData, duration);
    return data->duration;
}

extern "C" {

static VALUE duration_init(VM UNUSED(*vm), VALUE self, int arg_count, VALUE* arguments)
{
    std::chrono::nanoseconds value(0);
    if (arg_count >= 1) {
        value += date::years((int64_t)number_get_value(arguments[0]));
    }

    if (arg_count >= 2) {
        value += date::months((int64_t)number_get_value(arguments[1]));
    }

    if (arg_count >= 3) {
        value += date::days((int64_t)number_get_value(arguments[2]));
    }

    if (arg_count >= 4) {
        value += std::chrono::hours((int64_t)number_get_value(arguments[3]));
    }

    if (arg_count >= 5) {
        value += std::chrono::minutes((int64_t)number_get_value(arguments[4]));
    }

    if (arg_count >= 6) {
        value += std::chrono::seconds((int64_t)number_get_value(arguments[5]));
    }

    if (arg_count >= 7) {
        value += std::chrono::milliseconds((int64_t)number_get_value(arguments[6]));
    }
    DurationData *data = GET_NATIVE_INSTANCE_DATA(DurationData, self);
    data->duration = value;
    return NIL_VAL;
}

static VALUE duration_from_years(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    return duration_create(vm, date::years((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_months(VM *vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, date::months((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_days(VM *vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, date::days((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_hours(VM* vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, std::chrono::hours((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_minutes(VM *vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, std::chrono::minutes((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_seconds(VM* vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, std::chrono::seconds((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_milliseconds(VM* vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, std::chrono::milliseconds((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_operator_plus(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    DurationData *data = GET_NATIVE_INSTANCE_DATA(DurationData, OBJ_VAL(self));
    if (IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_DATETIME)) {
        auto datetime = datetime_get_value(arguments[0]);
        auto sum = data->duration + datetime.get_local_time();
        return create_datetime(vm, sum, datetime.get_time_zone());
    }
    else if (IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_DURATION)) {
        DurationData *rhs = GET_NATIVE_INSTANCE_DATA(DurationData, arguments[0]);
        auto result = data->duration + rhs->duration;
        return duration_create(vm, result.count());
    }
    return NIL_VAL;
}

VALUE duration_create(VM *vm, const int64_t count)
{
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(duration_class)));
    DurationData *result_data = GET_NATIVE_INSTANCE_DATA(DurationData, result);
    result_data->duration = std::chrono::nanoseconds(count);
    return result;
}

void init_duration(VM *vm)
{
    duration_class = defineNativeClass(vm, "Duration", NULL, NULL, NULL, NULL, CLS_DURATION, sizeof(DurationData), false);

    defineNativeMethod(vm, duration_class, &duration_init, "init", 0, false);

    defineNativeMethod(vm, duration_class, &duration_from_years, "from_years", 1, true);
    defineNativeMethod(vm, duration_class, &duration_from_months, "from_months", 1, true);
    defineNativeMethod(vm, duration_class, &duration_from_days, "from_days", 1, true);
    defineNativeMethod(vm, duration_class, &duration_from_hours, "from_hours", 1, true);
    defineNativeMethod(vm, duration_class, &duration_from_minutes, "from_minutes", 1, true);
    defineNativeMethod(vm, duration_class, &duration_from_seconds, "from_seconds", 1, true);
    defineNativeMethod(vm, duration_class, &duration_from_milliseconds, "from_milliseconds", 1, true);

    defineNativeOperator(vm, duration_class, &duration_operator_plus, 1, OPERATOR_PLUS);
}

}