#include "comet.h"
#include "comet_stdlib.h"
#include "list.h"

static VALUE klass;

typedef struct {
    ObjInstance obj;
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

void module_mark_contents(VALUE self)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, self);
    markValue(OBJ_VAL(data->main));
}

VALUE module_functions(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE result = list_create(vm);
    push(vm, result);
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, self);
    VALUE keys = list_create(vm);
    push(vm, keys);
    tableGetKeys(&data->obj.fields, vm, keys);
    VALUE iter = list_iterable_iterator(vm, keys, 0, NULL);
    push(vm, iter);
    VALUE has_next = list_iterator_has_next_p(vm, iter, 0, NULL);
    while (has_next == TRUE_VAL)
    {
        VALUE field = list_iterator_get_next(vm, iter, 0, NULL);
        push(vm, field);
        VALUE val;
        tableGet(&data->obj.fields, field, &val);
        if (IS_BOUND_METHOD(val) || IS_FUNCTION(val) || IS_CLOSURE(val) ||
            IS_NATIVE(val) || IS_NATIVE_METHOD(val))
        {
            list_add(vm, result, 1, &field);
        }
        pop(vm); // field
        has_next = list_iterator_has_next_p(vm, iter, 0, NULL);
    }

    pop(vm); // iter
    pop(vm); // keys
    return pop(vm); // result
}

VALUE module_fields(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE result = list_create(vm);
    push(vm, result);
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, self);
    VALUE keys = list_create(vm);
    push(vm, keys);
    tableGetKeys(&data->obj.fields, vm, keys);
    VALUE iter = list_iterable_iterator(vm, keys, 0, NULL);
    push(vm, iter);
    VALUE has_next = list_iterator_has_next_p(vm, iter, 0, NULL);
    while (has_next == TRUE_VAL)
    {
        VALUE field = list_iterator_get_next(vm, iter, 0, NULL);
        push(vm, field);
        VALUE val;
        tableGet(&data->obj.fields, field, &val);
        if (IS_INSTANCE(field) || IS_NATIVE_INSTANCE(field) || IS_NUMBER(field))
        {
            list_add(vm, result, 1, &field);
        }
        pop(vm); // field
        has_next = list_iterator_has_next_p(vm, iter, 0, NULL);
    }

    pop(vm); // iter
    pop(vm); // keys
    return pop(vm); // result
}

void init_module(VM *vm)
{
    klass = defineNativeClass(
        vm,
        "Module",
        &module_constructor,
        NULL,
        &module_mark_contents,
        "Object",
        CLS_MODULE,
        sizeof(module_data_t),
        true);
    defineNativeMethod(vm, klass, &module_functions, "functions", 0, false);
    defineNativeMethod(vm, klass, &module_fields, "fields", 0, false);
}