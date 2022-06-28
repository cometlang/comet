#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <date/date.h>
#include <date/tz.h>

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"
#include "datetime.hpp"

using namespace std::chrono;

typedef struct {
    ObjNativeInstance obj;
    date::zoned_time<nanoseconds> point;
} DateTimeData;

static date::year_month_day get_ymd(VALUE self)
{
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(self));
    auto local = data->point.get_local_time();
    auto dp = date::floor<date::days>(local);
    return date::year_month_day{dp};
}

static date::hh_mm_ss<milliseconds> get_time(VALUE self)
{
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(self));
    auto local = data->point.get_local_time();
    auto dp = date::floor<date::days>(local);
    date::year_month_day ymd{dp};
    return date::hh_mm_ss{date::floor<milliseconds>(local - dp)};
}

date::zoned_time<std::chrono::nanoseconds> datetime_get_value(VALUE datetime)
{
    DateTimeData *date = GET_NATIVE_INSTANCE_DATA(DateTimeData, datetime);
    return date->point;
}

extern "C" {

static VALUE datetime_class;

VALUE create_datetime(VM *vm, std::chrono::time_point<date::local_t, std::chrono::nanoseconds> time_point, const date::time_zone *time_zone)
{
    ObjNativeInstance *instance = (ObjNativeInstance *)newInstance(vm, AS_CLASS(datetime_class));
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(instance));
    data->point = date::zoned_time<std::chrono::nanoseconds>{time_zone, time_point};
    return OBJ_VAL(instance);
}

static VALUE datetime_year(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto ymd = get_ymd(self);

    return create_number(vm, (int)ymd.year());
}

static VALUE datetime_month(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto ymd = get_ymd(self);

    return create_number(vm, (unsigned)ymd.month());
}

static VALUE datetime_day(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto ymd = get_ymd(self);

    return create_number(vm, (unsigned)ymd.day());
}

static VALUE datetime_hours(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto time = get_time(self);

    return create_number(vm, time.hours().count());
}

static VALUE datetime_minutes(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto time = get_time(self);

    return create_number(vm, time.minutes().count());
}

static VALUE datetime_seconds(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto time = get_time(self);

    return create_number(vm, time.seconds().count());
}

static VALUE datetime_milliseconds(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    auto time = get_time(self);

    return create_number(vm, time.subseconds().count());
}

static VALUE datetime_static_now(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = (ObjNativeInstance *)newInstance(vm, AS_CLASS(klass));
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(instance));
    data->point = date::zoned_time{date::current_zone(), date::utc_clock::to_sys<nanoseconds>(date::utc_clock::now())};
    return OBJ_VAL(instance);
}

static VALUE datetime_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(self));

    auto local = data->point.get_local_time();
    auto dp = date::floor<date::days>(local);
    date::year_month_day ymd{dp};
    date::hh_mm_ss time{date::floor<milliseconds>(local - dp)};
    auto offset = data->point.get_info().offset;
    auto offset_hours = floor<hours>(offset);
    auto offset_mins = floor<minutes>(offset) - offset_hours;
    std::string sign = offset.count() < 0 ? "-" : "+";

    std::stringstream stream;
    stream
        << ymd << "T" << time
        << sign
        << std::setw(2) << std::setfill('0') << offset_hours.count()
        << ":"
        << std::setw(2) << std::setfill('0') << offset_mins.count();
    auto date_string = stream.str();
    return copyString(vm, date_string.c_str(), date_string.length());
}

static VALUE datetime_operator_minus(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(self));
    if (IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_DATETIME)) {
        DateTimeData *rhs = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(arguments[0]));
        auto difference = data->point.get_local_time() - rhs->point.get_local_time();
        return duration_create(vm, difference.count());
    }
    else if (IS_INSTANCE_OF_STDLIB_TYPE(arguments[0], CLS_DURATION)) {
        std::chrono::nanoseconds duration_value = duration_get_value(arguments[0]);
        return create_datetime(vm, data->point.get_local_time() - duration_value, data->point.get_time_zone());
    }
    return NIL_VAL;
}

void init_datetime(VM *vm)
{
    datetime_class = defineNativeClass(vm, "DateTime", NULL, NULL, NULL, NULL, CLS_DATETIME, sizeof(DateTimeData), false);
    defineNativeMethod(vm, datetime_class, &datetime_static_now, "now", 0, true);
    defineNativeMethod(vm, datetime_class, &datetime_to_string, "to_string", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_year, "year", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_month, "month", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_day, "day", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_hours, "hours", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_minutes, "minutes", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_seconds, "seconds", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_milliseconds, "milliseconds", 0, false);

    defineNativeOperator(vm, datetime_class, &datetime_operator_minus, 1, OPERATOR_MINUS);
}

}
