#include <date/date.h>
#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"

static VALUE duration_class;

typedef struct {
    ObjNativeInstance obj;
    std::chrono::nanoseconds duration;
} DurationData;

extern "C" {

VALUE duration_create(VM *vm, const int64_t count)
{
    VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(duration_class)));
    DurationData *result_data = GET_NATIVE_INSTANCE_DATA(DurationData, result);
    result_data->duration = std::chrono::nanoseconds(count);
    return result;
}


void init_duration(VM *vm)
{
    duration_class = defineNativeClass(vm, "Duration", NULL, NULL, NULL, CLS_DURATION, sizeof(DurationData), false);
}

}