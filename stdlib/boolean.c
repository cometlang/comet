#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64) 
#  include <string.h>
#  define strcasecmp _stricmp 
#  define strncasecmp _strnicmp 
#else
#  include <strings.h>
#endif


#include "cometlib.h"
#include "comet_stdlib.h"

typedef struct {
    ObjInstance obj;
    bool value;
} BooleanData_t;

static BooleanData_t _true;
static BooleanData_t _false;

ObjInstance *boolean_true = (ObjInstance *) &_true;
ObjInstance *boolean_false = (ObjInstance *) &_false;

static VALUE bool_class;

VALUE boolean_to_string(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    BooleanData_t *data = GET_NATIVE_INSTANCE_DATA(BooleanData_t, self);
    if (data->value)
        return copyString(vm, "true", 4);
    return copyString(vm, "false", 5);
}

VALUE boolean_parse(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    const char *to_comp = string_get_cstr(arguments[0]);
    if (strcasecmp(to_comp, "true") == 0) {
        return TRUE_VAL;
    }
    else if (strcasecmp(to_comp, "false") == 0) {
        return FALSE_VAL;
    }
    throw_exception_native(vm, "ArgumentException", "Can't parse %s to a boolean", to_comp);
    return NIL_VAL;
}

static void init_instance(VM *vm, BooleanData_t *instance, ObjClass *klass, bool value)
{
    instance->obj.obj.type = OBJ_NATIVE_INSTANCE;
#if !REF_COUNT_MEM_MANAGEMENT
    instance->obj.obj.isMarked = false; // doesn't matter, it's static memory anyway
#endif
    instance->obj.klass = klass;
    instance->value = value;
    initTable(&instance->obj.fields);
    if (value)
        push(vm, copyString(vm, "true", 4));
    else
        push(vm, copyString(vm, "false", 5));
    addGlobal(peek(vm, 0), OBJ_VAL(instance));
    pop(vm);
}

bool bool_get_value(VALUE value)
{
    DEBUG_ASSERT(instanceof(value, bool_class));
    BooleanData_t *data = GET_NATIVE_INSTANCE_DATA(BooleanData_t, value);
    return (bool)data->value;
}

bool bool_is_falsey(VALUE value)
{
    if (IS_NIL(value))
        return true;
    if (instanceof(value, bool_class) && !bool_get_value(value))
        return true;

    return false;
}

void init_boolean(VM *vm)
{
    bool_class = defineNativeClass(vm, "Boolean", NULL, NULL, NULL, NULL, CLS_BOOLEAN, sizeof(BooleanData_t), true);
    defineNativeMethod(vm, bool_class, &boolean_to_string, "to_string", 0, false);
    defineNativeMethod(vm, bool_class, &boolean_parse, "parse", 1, true);
    init_instance(vm, &_true, AS_CLASS(bool_class), true);
    init_instance(vm, &_false, AS_CLASS(bool_class), false);
}