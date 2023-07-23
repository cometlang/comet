#ifndef _datetime_hpp_
#define _datetime_hpp_
#include <chrono>

#ifndef WIN32
#include "date/date.h"
#include "date/tz.h"
#endif
#include "comet.h"

extern "C" {
#ifdef WIN32
    VALUE create_datetime(VM *vm, std::chrono::time_point<std::chrono::local_t, std::chrono::nanoseconds> time_point, const std::chrono::time_zone *time_zone);
#else
    VALUE create_datetime(VM* vm, std::chrono::time_point<date::local_t, std::chrono::nanoseconds> time_point, const date::time_zone* time_zone);
#endif
}

#ifdef WIN32
std::chrono::zoned_time<std::chrono::nanoseconds> datetime_get_value(VALUE datetime);
#else
date::zoned_time<std::chrono::nanoseconds> datetime_get_value(VALUE datetime);
#endif
std::chrono::nanoseconds duration_get_value(VALUE duration);

#endif