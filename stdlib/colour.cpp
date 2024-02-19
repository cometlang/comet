#include "comet_stdlib.h"
#include "colour.h"
#include "math.h"
#include <string>
#include <sstream>


static VALUE colourClass = NIL_VAL;
static VALUE colour_category = NIL_VAL;

#define COLOUR_CAT_BLACK 0
#define COLOUR_CAT_WHITE 1
#define COLOUR_CAT_GREY 2
#define COLOUR_CAT_RED 3
#define COLOUR_CAT_ORANGE 4
#define COLOUR_CAT_YELLOW 5
#define COLOUR_CAT_GREEN 6
#define COLOUR_CAT_CYAN 7
#define COLOUR_CAT_BLUE 8
#define COLOUR_CAT_MAGENTA 9


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
    stream << "(" << data->r << ", " << data->g << ", " << data->b << ")";
    std::string result = stream.str();
    return copyString(vm, result.c_str(), result.length());
}

static VALUE colour_categorise(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ColourData_t* data = GET_NATIVE_INSTANCE_DATA(ColourData_t, self);
    double r = data->r / 255.0;
    double g = data->g / 255.0;
    double b = data->b / 255.0;

    double max = fmax(fmax(r, g), b);
    double min = fmin(fmin(r, g), b);

    double l = (max + min) / 2.0;
    double h, s;
    if (max == min) {
        h = s = 0; // achromatic
    }
    else {
        double d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
        if (max == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (max == g) h = (b - r) / d + 2;
        else if (max == b) h = (r - g) / d + 4;
        h *= 60;
        if (h < 0) {
            h += 360;
        }
    }

    if (l < 0.12)
        return enum_get_from_value(colour_category, COLOUR_CAT_BLACK);
    if (l > 0.98)
        return enum_get_from_value(colour_category, COLOUR_CAT_WHITE);
    if (s < 0.2)
        return enum_get_from_value(colour_category, COLOUR_CAT_GREY);
    if (h < 15)
        return enum_get_from_value(colour_category, COLOUR_CAT_RED);
    if (h < 45)
        return enum_get_from_value(colour_category, COLOUR_CAT_ORANGE);
    if (h < 65)
        return enum_get_from_value(colour_category, COLOUR_CAT_YELLOW);
    if (h < 170)
        return enum_get_from_value(colour_category, COLOUR_CAT_GREEN);
    if (h < 190)
        return enum_get_from_value(colour_category, COLOUR_CAT_CYAN);
    if (h < 270)
        return enum_get_from_value(colour_category, COLOUR_CAT_BLUE);
    if (h < 335)
        return enum_get_from_value(colour_category, COLOUR_CAT_MAGENTA);

    return enum_get_from_value(colour_category, COLOUR_CAT_RED);
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
        defineNativeMethod(vm, colourClass, &colour_categorise, "categorise", 0, false);

        colour_category = enum_create(vm);
        push(vm, colour_category);
        addGlobal(copyString(vm, "COLOUR_CATEGORY", 15), colour_category);
        enum_add_value(vm, colour_category, "BLACK", COLOUR_CAT_BLACK);
        enum_add_value(vm, colour_category, "WHITE", COLOUR_CAT_WHITE);
        enum_add_value(vm, colour_category, "GREY", COLOUR_CAT_GREY);
        enum_add_value(vm, colour_category, "RED", COLOUR_CAT_RED);
        enum_add_value(vm, colour_category, "ORANGE", COLOUR_CAT_ORANGE);
        enum_add_value(vm, colour_category, "YELLOW", COLOUR_CAT_YELLOW);
        enum_add_value(vm, colour_category, "GREEN", COLOUR_CAT_GREEN);
        enum_add_value(vm, colour_category, "CYAN", COLOUR_CAT_CYAN);
        enum_add_value(vm, colour_category, "BLUE", COLOUR_CAT_BLUE);
        enum_add_value(vm, colour_category, "MAGENTA", COLOUR_CAT_MAGENTA);
        pop(vm);
    }
}