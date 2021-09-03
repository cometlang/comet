#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <date/date.h>
#include <date/tz.h>

extern "C" {

#include "cometlib.h"
#include "comet_stdlib.h"

using namespace std::chrono;

typedef struct {
    date::zoned_time<nanoseconds> point;
} DateTimeData;

static void *datetime_constructor(void)
{
    DateTimeData *data = ALLOCATE(DateTimeData, 1);
    return data;
}

static void datetime_destructor(void *data)
{
    FREE(DateTimeData, data);
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
    auto y = ymd.year();
    auto m = ymd.month();
    auto d = ymd.day();
    auto h = time.hours();
    auto M = time.minutes();
    auto s = time.seconds();
    auto ms = time.subseconds();

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

void init_datetime(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "DateTime", &datetime_constructor, &datetime_destructor, NULL, CLS_DATETIME);
    defineNativeMethod(vm, klass, &datetime_static_now, "now", 0, true);
    defineNativeMethod(vm, klass, &datetime_to_string, "to_string", 0, false);
}

}
