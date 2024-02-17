#include "comet_stdlib.h"
#include "colour.h"
#include <string>
#include <sstream>

#include "stdio.h"

static VALUE colourClass = NIL_VAL;

static VALUE colour_init(VM UNUSED(*vm), VALUE self, int arg_count, VALUE *arguments)
{
    ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, self);
    data->r = 0;
    data->g = 0;
    data->b = 0;
    if (arg_count >= 1) {
        data->r = (uint8_t) number_get_value(arguments[0]);
    }
    if (arg_count >= 2) {
        data->g = (uint8_t) number_get_value(arguments[1]);
    }
    if (arg_count >= 3) {
        data->b = (uint8_t) number_get_value(arguments[2]);
    }
    printf("Colour: (%d, %d, %d)\n", data->r, data->g, data->b);
    return NIL_VAL;
}

static VALUE colour_red(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, self);
    return create_number(vm, data->r);
}

static VALUE colour_green(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, self);
    return create_number(vm, data->g);
}

static VALUE colour_blue(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, self);
    return create_number(vm, data->b);
}

static VALUE colour_to_string(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, self);
    std::stringstream stream;
    stream << "colour: " << data->r << ", " << data->g << ", " << data->b;
    std::string result = stream.str();
    return copyString(vm, result.c_str(), result.length());
}

extern "C" {
    VALUE colour_create(VM *vm, uint8_t r, uint8_t g, uint8_t b)
    {
        VALUE result = OBJ_VAL(newInstance(vm, AS_CLASS(colourClass)));
        push(vm, result);
        ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, result);
        data->r = r;
        data->g = g;
        data->b = b;
        return pop(vm);
    }

    void init_colour(VM *vm)
    {
        colourClass = defineNativeClass(vm, "Colour", NULL, NULL, NULL, "Object", CLS_COLOUR, sizeof(ColourData_t), false);
        defineNativeMethod(vm, colourClass, &colour_init, "init", 0, false);
        defineNativeMethod(vm, colourClass, &colour_red, "red", 0, false);
        defineNativeMethod(vm, colourClass, &colour_green, "green", 0, false);
        defineNativeMethod(vm, colourClass, &colour_blue, "blue", 0, false);
        defineNativeMethod(vm, colourClass, &colour_to_string, "to_string", 0, false);
    }
}