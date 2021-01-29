#include "cometlib.h"
#include "comet_stdlib.h"

VALUE enum_class;
VALUE enum_value_class;

typedef struct {
    uint64_t current_value;
    int count;
    int capacity;
    VALUE *entries;
} EnumData;

typedef struct {
    uint64_t value;
    VALUE name;
} EnumValueData;

static void *enum_constructor(void)
{
    EnumData *data = ALLOCATE(EnumData, 1);
    data->current_value = 0;
    data->count = 0;
    data->capacity = 0;
    data->entries = NULL;
    return data;
}

static void enum_destructor(void *to_destruct)
{
    EnumData *data = (EnumData *) to_destruct;
    FREE_ARRAY(EnumData, data->entries, data->count);
    data->count = 0;
    data->entries = NULL;
    data->capacity = 0;
    FREE(EnumData, data);
}

static VALUE enum_parse(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    // handle string names, string ints and int ints.
    return NIL_VAL;
}

static VALUE enum_add(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

static VALUE enum_iterator(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

static VALUE enum_contains_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}

static VALUE enum_empty_q(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return FALSE_VAL;
}


static void *enumvalue_constructor(void)
{
    EnumValueData *data = ALLOCATE(EnumValueData, 1);
    data->name = NIL_VAL;
    data->value = 0;
    return data;
}

static void enumvalue_destructor(void *data)
{
    FREE(EnumValueData, data);
}

static VALUE enumvalue_init(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

static VALUE enumvalue_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_enum(VM *vm)
{
    enum_class = defineNativeClass(vm, "Enum", &enum_constructor, &enum_destructor, "Iterable");
    defineNativeMethod(vm, enum_class, &enum_parse, "parse", 1, true);
    defineNativeMethod(vm, enum_class, &enum_add, "add", 1, false);
    defineNativeMethod(vm, enum_class, &enum_iterator, "iterator", 0, false);
    defineNativeMethod(vm, enum_class, &enum_contains_q, "contains?", 1, false);
    defineNativeMethod(vm, enum_class, &enum_empty_q, "empty?", 0, false);

    // This will require me to call 'super.init()' somehow...
    enum_value_class = defineNativeClass(vm, "EnumValue", &enumvalue_constructor, &enumvalue_destructor, "Number");
    defineNativeMethod(vm, enum_value_class, &enumvalue_init, "init", 2, false);
    defineNativeMethod(vm, enum_value_class, &enumvalue_to_string, "to_string", 0, false);
}