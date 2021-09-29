#include "comet.h"
#include "comet_stdlib.h"

static VALUE klass;

typedef struct {
    ObjNativeInstance obj;
    const char *filename;
    bool initialized;
    ObjFunction *main;
} module_data_t;

static void module_constructor(void *instanceData)
{
    module_data_t *data = (module_data_t *)instanceData;
    data->filename = NULL;
    data->initialized = false;
    data->main = NULL;
}

bool module_is_initialized(VALUE module)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, module);
    return data->initialized;
}

void module_set_initialized(VALUE module)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, module);
    data->initialized = true;
}

const char *module_filename(VALUE module)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, module);
    return data->filename;
}

void module_set_main(VALUE module, ObjFunction *function)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, module);
    data->main = function;
}

ObjFunction *module_get_main(VALUE module)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, module);
    return data->main;
}

VALUE module_create(VM *vm, const char *filename)
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(klass)));
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, instance);
    data->filename = filename;
    return instance;
}

void init_module(VM *vm)
{
    klass = defineNativeClass(vm, "Module", &module_constructor, NULL, "Object", CLS_MODULE, sizeof(module_data_t), true);
}