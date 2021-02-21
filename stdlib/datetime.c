#include "cometlib.h"

static VALUE datetime_class;


void init_datetime(VM *vm)
{
    datetime_class = defineNativeClass(vm, "DateTime", NULL, NULL, NULL, CLS_DATETIME);
}
