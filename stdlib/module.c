#include "comet.h"
#include "comet_stdlib.h"
#include "list.h"

static VALUE klass;

static void addExecutionCountsForFunctions(VM *vm, VALUE hash, Table *functions);

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
            list_add(vm, result, 1, &val);
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

VALUE module_filename_field(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, self);
    return copyString(vm, data->filename, strlen(data->filename));
}

VALUE module_index(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, self);
    VALUE result;
    if (tableGet(&data->obj.fields, arguments[0], &result))
    {
        return result;
    }
    else if (tableGet(&data->obj.klass->methods, arguments[0], &result))
    {
        return result;
    }
    return NIL_VAL;
}

static void addExecutionCountsForFunction(VM *vm, VALUE hash, ObjFunction *function)
{
    VALUE args[2];
    VALUE result = hash_create(vm);
    push(vm, result);
    VALUE function_name = function->name;
    if (function_name == NIL_VAL)
    {
        function_name = common_strings[STRING_SCRIPT];
    }
    args[0] = function_name;
    args[1] = result;
    hash_add(vm, hash, 2, args);
    Chunk *chunk = &function->chunk;
    for (int i = 0; i < chunk->count; i++)
    {
        int line = chunk->lines[i];
        VALUE line_no = create_number(vm, line);
        push(vm, line_no);
        VALUE ex_no = create_number(vm, chunk->execution_counts[i]);
        push(vm, ex_no);
        VALUE line_val = hash_get(vm, result, 1, &line_no);
        if (line_val == NIL_VAL || chunk->execution_counts[i] > number_get_value(line_val))
        {
            args[0] = line_no;
            args[1] = ex_no;
            hash_add(vm, result, 2, args);
        }
        pop(vm); // ex_no
        pop(vm); // line_no
    }
    pop(vm); // result
}

static void addExecutionCountsForValue(VM *vm, VALUE hash, VALUE val)
{
    if (IS_BOUND_METHOD(val))
    {
        addExecutionCountsForFunction(vm, hash, AS_BOUND_METHOD(val)->method->function);
    }
    else if (IS_FUNCTION(val))
    {
        addExecutionCountsForFunction(vm, hash, AS_FUNCTION(val));
    }
    else if (IS_CLOSURE(val))
    {
        addExecutionCountsForFunction(vm, hash, AS_CLOSURE(val)->function);
    }
    else if (IS_CLASS(val))
    {
        addExecutionCountsForFunctions(vm, hash, &AS_CLASS(val)->methods);
        addExecutionCountsForFunctions(vm, hash, &AS_CLASS(val)->staticMethods);
    }
}

static void addExecutionCountsForFunctions(VM *vm, VALUE hash, Table *functions)
{
    VALUE functions_list = list_create(vm);
    push(vm, functions_list);
    tableGetValues(functions, vm, functions_list);
    VALUE functions_iter = list_iterable_iterator(vm, functions_list, 0, NULL);
    push(vm, functions_iter);
    VALUE has_next = list_iterator_has_next_p(vm, functions_iter, 0, NULL);
    while (has_next == TRUE_VAL)
    {
        VALUE func = list_iterator_get_next(vm, functions_iter, 0, NULL);
        addExecutionCountsForValue(vm, hash, func);
        has_next = list_iterator_has_next_p(vm, functions_iter, 0, NULL);
    }
    pop(vm); // functions_iter
    pop(vm); // functions_list
}

VALUE module_get_execution_counts(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    module_data_t *data = GET_NATIVE_INSTANCE_DATA(module_data_t, self);
    VALUE result = hash_create(vm);
    push(vm, result);
    VALUE fields_list = list_create(vm);
    push(vm, fields_list);
    tableGetValues(&data->obj.fields, vm, fields_list);
    VALUE fields_iter = list_iterable_iterator(vm, fields_list, 0, NULL);
    push(vm, fields_iter);
    VALUE has_next = list_iterator_has_next_p(vm, fields_iter, 0, NULL);
    while (has_next == TRUE_VAL)
    {
        VALUE val = list_iterator_get_next(vm, fields_iter, 0, NULL);
        addExecutionCountsForValue(vm, result, val);
        has_next = list_iterator_has_next_p(vm, fields_iter, 0, NULL);
    }
    addExecutionCountsForFunction(vm, result, data->main);
    pop(vm); // fields_iter
    pop(vm); // fields_list
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
    defineNativeMethod(vm, klass, &module_filename_field, "filename", 0, false);
    defineNativeMethod(vm, klass, &module_get_execution_counts, "get_execution_counts", 0, false);

    defineNativeOperator(vm, klass, &module_index, 1, OPERATOR_INDEX);
}