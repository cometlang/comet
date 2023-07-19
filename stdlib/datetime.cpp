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
    ObjInstance obj;
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
    ObjInstance *instance = (ObjInstance *)newInstance(vm, AS_CLASS(datetime_class));
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
    ObjInstance *instance = (ObjInstance *)newInstance(vm, AS_CLASS(klass));
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(instance));
    data->point = date::zoned_time{date::current_zone(), date::utc_clock::to_sys<nanoseconds>(date::utc_clock::now())};
    return OBJ_VAL(instance);
}

static VALUE datetime_init(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    int year = 0;
    unsigned int month = 0, day = 0, hours = 0, mins = 0, seconds = 0, millis = 0;
    if (arg_count > 0) year = number_get_value(arguments[0]);
    if (arg_count > 1) month = number_get_value(arguments[1]);
    if (arg_count > 2) day = number_get_value(arguments[2]);
    if (arg_count > 3) hours = number_get_value(arguments[3]);
    if (arg_count > 4) mins = number_get_value(arguments[4]);
    if (arg_count > 5) seconds = number_get_value(arguments[5]);
    if (arg_count > 6) millis = number_get_value(arguments[6]);
    std::chrono::system_clock::time_point dateTime =
        date::sys_days(date::year(year) / date::month(month) / date::day(day))
        + std::chrono::hours(hours)
        + std::chrono::minutes(mins)
        + std::chrono::seconds(seconds)
        + std::chrono::milliseconds(millis);
    DateTimeData *data = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(self));
    data->point = date::zoned_time{dateTime};
    return datetime_static_now(vm, OBJ_VAL(AS_INSTANCE(self)->klass), 0, NULL);
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

static VALUE datetime_static_parse(VM *vm, VALUE UNUSED(klass), int arg_count, VALUE *arguments)
{
    const char *format = "%Y-%m-%dT%H:%M:%6S%z";
    if (arg_count == 2)
        format = string_get_cstr(arguments[1]);
    std::stringstream to_parse(string_get_cstr(arguments[0]));
    std::chrono::time_point<date::local_t, std::chrono::nanoseconds> tp;
    std::chrono::minutes offset = std::chrono::minutes::zero();
    std::string tz;
    to_parse >> date::parse(format, tp, tz, offset);
    const date::tzdb& zones = date::get_tzdb();
    if (tz.empty())
    {
        std::chrono::seconds offset_s = duration_cast<std::chrono::seconds>(offset);
        for (auto& zone : zones.zones) {
            const auto& info = zone.get_info(tp);
            if (info.first.offset == offset_s || info.second.offset == offset_s)
            {
                tz = zone.name();
            }
        }
    }
    return create_datetime(vm, tp, date::locate_zone(tz));
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

static VALUE datetime_operator_equals(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    DateTimeData *lhs = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(self));
    DateTimeData *rhs = GET_NATIVE_INSTANCE_DATA(DateTimeData, OBJ_VAL(arguments[0]));
    if (lhs->point == rhs->point)
        return TRUE_VAL;
    return FALSE_VAL;
}

void init_datetime(VM *vm)
{
    datetime_class = defineNativeClass(vm, "DateTime", NULL, NULL, NULL, NULL, CLS_DATETIME, sizeof(DateTimeData), false);
    defineNativeMethod(vm, datetime_class, &datetime_static_now, "now", 0, true);
    defineNativeMethod(vm, datetime_class, &datetime_init, "init", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_to_string, "to_string", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_static_parse, "parse", 1, true);
    defineNativeMethod(vm, datetime_class, &datetime_year, "year", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_month, "month", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_day, "day", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_hours, "hours", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_minutes, "minutes", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_seconds, "seconds", 0, false);
    defineNativeMethod(vm, datetime_class, &datetime_milliseconds, "milliseconds", 0, false);

    defineNativeOperator(vm, datetime_class, &datetime_operator_minus, 1, OPERATOR_MINUS);
    defineNativeOperator(vm, datetime_class, &datetime_operator_equals, 1, OPERATOR_EQUALS);
}

}
