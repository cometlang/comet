#include "comet.h"
#include "util.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


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

VALUE file_close(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *) instance->data;
    if (data->fp != NULL)
        fclose(data->fp);
    return NIL_VAL;
}

VALUE file_write(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *) instance->data;
    int result = fprintf(data->fp, "%s", AS_STRING(arguments[0])->chars);
    return NUMBER_VAL(result);
}

VALUE file_read(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData UNUSED(*data) = (FileData *) instance->data;
    return NIL_VAL;
}

VALUE file_read_lines(VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData UNUSED(*data) = (FileData *) instance->data;
    return NIL_VAL;
}

VALUE file_flush(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *) instance->data;
    int result = fflush(data->fp);
    if (result == EOF)
    {
        fprintf(stderr, "%s", strerror(errno));
    }
    return NIL_VAL;
}

VALUE file_sync(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *) instance->data;
    int fd = fileno(data->fp);
    fsync(fd);
    return NIL_VAL;
}

VALUE file_static_exists_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(AS_STRING(arguments[0])->chars, &statbuf) == 0)
        return BOOL_VAL(true);
    return BOOL_VAL(false);
}

VALUE file_static_directory_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(AS_STRING(arguments[0])->chars, &statbuf) == 0)
        return BOOL_VAL(statbuf.st_mode & __S_IFDIR);
    return BOOL_VAL(false);
}

VALUE file_static_file_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(AS_STRING(arguments[0])->chars, &statbuf) == 0)
        return BOOL_VAL(statbuf.st_mode & __S_IFREG);
    return BOOL_VAL(false);
}

void init_file(void)
{
    VALUE klass = defineNativeClass("File", &file_constructor, &file_destructor, "Object");
    defineNativeMethod(klass, file_static_open, "open", true);
    defineNativeMethod(klass, file_close, "close", false);
    defineNativeMethod(klass, file_write, "write", false);
    defineNativeMethod(klass, file_sync, "sync", false);
    defineNativeMethod(klass, file_flush, "flush", false);
    defineNativeMethod(klass, file_static_exists_q, "exists?", true);
    defineNativeMethod(klass, file_static_directory_q, "directory?", true);
    defineNativeMethod(klass, file_static_file_q, "file?", true);
}