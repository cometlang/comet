#include "comet.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct fileData
{
    FILE *fp;
} FileData;

void *file_constructor(void)
{
    FileData *data = (FileData *) malloc(sizeof(FileData));
    data->fp = NULL;
    return (void *) data;
}

void file_destructor(void *data)
{
    FileData *file_data = (FileData *) data;
    if (file_data->fp != NULL)
    {
        fflush(file_data->fp);
        fclose(file_data->fp);
    }
    free(file_data);
}

VALUE file_static_open(VALUE klass, int arg_count, VALUE *arguments)
{
    ObjNativeInstance *instance = (ObjNativeInstance *) newInstance(AS_CLASS(klass));
    if (arg_count != 2)
    {
        fprintf(stderr, "Wrong number of arguments: got %d, needed 2\n", arg_count);
        // Throw an exception
        return NIL_VAL;
    }
    const char *path = AS_STRING(arguments[0])->chars;
    const char *mode = AS_STRING(arguments[1])->chars;
    FILE *fp = fopen(path, mode);
    //should probably check for NULL and free the object, returning NIL, or throwing an exception or something
    ((FileData *) instance->data)->fp = fp;
    return OBJ_VAL(instance);
}

VALUE file_close(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_write(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_read(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_read_lines(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_flush(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_sync(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_static_exists_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_static_directory_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_static_file_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_file(void)
{
    VALUE klass = defineNativeClass("File", &file_constructor, &file_destructor, "Object");
    defineNativeMethod(klass, file_static_open, "open", true);
}