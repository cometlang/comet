#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include "stb_image.h"
#include "stb_image_write.h"
#include "comet.h"
#include "comet_stdlib.h"
#include "cometlib.h"
#include "colour.h"

#define IMAGE_TYPE_PNG 0
#define IMAGE_TYPE_JPEG 1
#define IMAGE_TYPE_BMP 2

#define IMAGE_COLOUR_SPACE_BYTES 3
#define JPEG_IMAGE_QUALITY 90

typedef struct
{
    ObjInstance obj;
    int width;
    int height;
    int channels;
    uint8_t *buffer;
} ImageData;

static void image_constructor(void* instanceData)
{
    ImageData* data = (ImageData*)instanceData;
    data->buffer = NULL;
    data->width = 0;
    data->height = 0;
    data->channels = IMAGE_COLOUR_SPACE_BYTES;
}

static void image_destructor(void* instanceData)
{
    ImageData* data = (ImageData*)instanceData;
    if (data->buffer != NULL)
    {
        free(data->buffer);
        data->buffer = NULL;
        data->height = 0;
        data->width = 0;
    }
}

static VALUE image_init(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ImageData* data = GET_NATIVE_INSTANCE_DATA(ImageData, self);
    data->width = (int)number_get_value(arguments[0]);
    data->height = (int)number_get_value(arguments[1]);
    data->buffer = (uint8_t*)malloc(data->width * data->height * data->channels);
    return NIL_VAL;
}

static VALUE image_set_pixel(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ImageData* data = GET_NATIVE_INSTANCE_DATA(ImageData, self);
    int x = (int)number_get_value(arguments[0]);
    int y = (int)number_get_value(arguments[1]);
    ColourData_t* colourData = GET_NATIVE_INSTANCE_DATA(ColourData_t, arguments[2]);
    int index = (y * (data->width * data->channels)) + (x * data->channels);
    data->buffer[index + 0] = colourData->r;
    data->buffer[index + 1] = colourData->g;
    data->buffer[index + 2] = colourData->b;
    return NIL_VAL;
}

static VALUE image_get_pixel(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ImageData* data = GET_NATIVE_INSTANCE_DATA(ImageData, self);
    int x = (int)number_get_value(arguments[0]);
    int y = (int)number_get_value(arguments[1]);
    int index = (y * (data->width * data->channels)) + (x * data->channels);
    uint8_t r = data->buffer[index + 0];
    uint8_t g = data->buffer[index + 1];
    uint8_t b = data->buffer[index + 2];
    return colour_create(vm, r, g, b);
}

static VALUE image_write_to_file(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    ImageData* data = GET_NATIVE_INSTANCE_DATA(ImageData, self);
    const char *filename = string_get_cstr(arguments[0]);
    uint64_t format = enumvalue_get_value(arguments[1]);
    stbi_flip_vertically_on_write(true);
    if (format == IMAGE_TYPE_PNG)
    {
        stbi_write_png(filename, data->width, data->height, data->channels, data->buffer, data->width * data->channels);
    }
    else if (format == IMAGE_TYPE_JPEG)
    {
        stbi_write_jpg(filename, data->width, data->height, data->channels, data->buffer, JPEG_IMAGE_QUALITY);
    }
    else if (format == IMAGE_TYPE_BMP)
    {
        stbi_write_bmp(filename, data->width, data->height, data->channels, data->buffer);
    }
    return NIL_VAL;
}

static VALUE image_width(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ImageData *data = GET_NATIVE_INSTANCE_DATA(ImageData, self);
    return create_number(vm, data->width);
}

static VALUE image_height(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ImageData *data = GET_NATIVE_INSTANCE_DATA(ImageData, self);
    return create_number(vm, data->height);
}

static VALUE image_read(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE *arguments)
{
    int x, y, channels;
    ImageData *image = (ImageData *) newInstance(vm, AS_CLASS(klass));
    push(vm, OBJ_VAL(image));
    stbi_uc *data = stbi_load(string_get_cstr(arguments[0]), &x, &y, &channels, IMAGE_COLOUR_SPACE_BYTES);
    image_constructor(image);
    image->buffer = data;
    image->width = x;
    image->height = y;
    image->channels = channels;
    return pop(vm);
}

void init_image(VM* vm)
{
    VALUE klass = defineNativeClass(vm, "Image", image_constructor, image_destructor, NULL, NULL, CLS_IMAGE, sizeof(ImageData), false);
    defineNativeMethod(vm, klass, &image_init, "init", 2, false);
    defineNativeMethod(vm, klass, &image_set_pixel, "set_pixel", 3, false);
    defineNativeMethod(vm, klass, &image_get_pixel, "get_pixel", 2, false);
    defineNativeMethod(vm, klass, &image_write_to_file, "write_to_file", 2, false);
    defineNativeMethod(vm, klass, &image_width, "width", 0, false);
    defineNativeMethod(vm, klass, &image_height, "height", 0, false);
    defineNativeMethod(vm, klass, &image_read, "read", 1, true);

    VALUE image_formats = enum_create(vm);
    push(vm, image_formats);
    addGlobal(copyString(vm, "IMAGE_FORMAT", 12), image_formats);
    enum_add_value(vm, image_formats, "PNG", IMAGE_TYPE_PNG);
    enum_add_value(vm, image_formats, "JPEG", IMAGE_TYPE_JPEG);
    enum_add_value(vm, image_formats, "BMP", IMAGE_TYPE_BMP);
    pop(vm);
}