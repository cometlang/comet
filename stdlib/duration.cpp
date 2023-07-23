#include <chrono>
#include <iostream>
#include <sstream>
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

static void set_duration_properties(VM *vm, VALUE self, std::chrono::nanoseconds& value)
{
    setNativeProperty(vm, self, "nanoseconds", create_number(vm, value.count()));
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(value);
    setNativeProperty(vm, self, "milliseconds", create_number(vm, milliseconds.count()));
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(value);
    setNativeProperty(vm, self, "seconds", create_number(vm, seconds.count()));
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(value);
    setNativeProperty(vm, self, "minutes", create_number(vm, minutes.count()));
    auto hours = std::chrono::duration_cast<std::chrono::hours>(value);
    setNativeProperty(vm, self, "hours", create_number(vm, hours.count()));
    auto days = std::chrono::duration_cast<std::chrono::days>(value);
    setNativeProperty(vm, self, "days", create_number(vm, days.count()));
}

extern "C" {

static VALUE duration_init(VM UNUSED(*vm), VALUE self, int arg_count, VALUE* arguments)
{
    std::chrono::nanoseconds value(0);
    if (arg_count >= 1) {
        value += std::chrono::years((int64_t)number_get_value(arguments[0]));
    }

    if (arg_count >= 2) {
        value += std::chrono::months((int64_t)number_get_value(arguments[1]));
    }

    if (arg_count >= 3) {
        value += std::chrono::days((int64_t)number_get_value(arguments[2]));
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
    set_duration_properties(vm, self, value);
    return NIL_VAL;
}

static VALUE duration_from_years(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    return duration_create(vm, std::chrono::years((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_months(VM *vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, std::chrono::months((int64_t)number_get_value(arguments[0])).count());
}

static VALUE duration_from_days(VM *vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    return duration_create(vm, std::chrono::days((int64_t)number_get_value(arguments[0])).count());
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

static VALUE duration_operator_plus(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
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

static VALUE duration_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    DurationData *data = GET_NATIVE_INSTANCE_DATA(DurationData, OBJ_VAL(self));
    std::ostringstream out;
    out << std::chrono::nanoseconds(data->duration).count() << "ns";
    auto output = out.str();
    return copyString(vm, output.c_str(), output.length());
}

VALUE duration_create(VM *vm, const int64_t count)
{
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(duration_class)));
    DurationData *result_data = GET_NATIVE_INSTANCE_DATA(DurationData, result);
    result_data->duration = std::chrono::nanoseconds(count);
    set_duration_properties(vm, result, result_data->duration);
    return result;
}

void init_duration(VM *vm)
{
    duration_class = defineNativeClass(vm, "Duration", NULL, NULL, NULL, NULL, CLS_DURATION, sizeof(DurationData), false);

    defineNativeMethod(vm, duration_class, &duration_init, "init", 0, false);
    defineNativeMethod(vm, duration_class, &duration_to_string, "to_string", 0, false);

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