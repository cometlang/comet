#ifndef _datetime_hpp_
#define _datetime_hpp_
#include <chrono>
#include "date/date.h"
#include "date/tz.h"
#include "comet.h"

extern "C" {

    VALUE create_datetime(VM *vm, std::chrono::time_point<date::local_t, std::chrono::nanoseconds> time_point, const date::time_zone *time_zone);
}

date::zoned_time<std::chrono::nanoseconds> datetime_get_value(VALUE datetime);
std::chrono::nanoseconds duration_get_value(VALUE duration);

#endif