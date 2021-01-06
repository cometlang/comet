#include "comet.h"

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
    FileData *data = ALLOCATE(FileData, 1);
    data->fp = NULL;
    return (void *)data;
}

void file_destructor(void *data)
{
    FileData *file_data = (FileData *)data;
    if (file_data->fp != NULL)
    {
        fflush(file_data->fp);
        fclose(file_data->fp);
    }
    FREE(FileData, file_data);
}

VALUE file_static_open(VALUE klass, int arg_count, VALUE *arguments)
{
    ObjNativeInstance *instance = (ObjNativeInstance *)newInstance(AS_CLASS(klass));
    if (arg_count != 2)
    {
        fprintf(stderr, "Wrong number of arguments: got %d, needed 2\n", arg_count);
        // Throw an exception
        return NIL_VAL;
    }
    const char *path = get_cstr(arguments[0]);
    const char *mode = get_cstr(arguments[1]);
    FILE *fp = fopen(path, mode);
    //should probably check for NULL and free the object, returning NIL, or throwing an exception or something
    ((FileData *)instance->data)->fp = fp;
    return OBJ_VAL(instance);
}

VALUE file_close(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;
    if (data->fp != NULL)
    {
        fclose(data->fp);
        data->fp = NULL;
    }
    return NIL_VAL;
}

VALUE file_write(VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;
    int result = fprintf(data->fp, "%s", get_cstr(arguments[0]));
    return NUMBER_VAL(result);
}

VALUE file_read(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;

    fseek(data->fp, 0L, SEEK_END);
    size_t fileSize = ftell(data->fp);
    rewind(data->fp);

    char *buffer = (char *) malloc(sizeof(char) * (fileSize + 1));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, data->fp);
    buffer[bytesRead] = '\0';

    return OBJ_VAL(takeString(buffer, fileSize));
}

VALUE file_flush(VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;
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
    FileData *data = (FileData *)instance->data;
    int fd = fileno(data->fp);
    fsync(fd);
    return NIL_VAL;
}

VALUE file_static_exists_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(get_cstr(arguments[0]), &statbuf) == 0)
        return BOOL_VAL(true);
    return BOOL_VAL(false);
}

VALUE file_static_directory_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(get_cstr(arguments[0]), &statbuf) == 0)
        return BOOL_VAL(statbuf.st_mode & __S_IFDIR);
    return BOOL_VAL(false);
}

VALUE file_static_file_q(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(get_cstr(arguments[0]), &statbuf) == 0)
        return BOOL_VAL(statbuf.st_mode & __S_IFREG);
    return BOOL_VAL(false);
}

VALUE file_static_read_all_lines(VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_file(void)
{
    VALUE klass = defineNativeClass("File", &file_constructor, &file_destructor, "Object");
    defineNativeMethod(klass, &file_static_open, "open", true);
    defineNativeMethod(klass, &file_close, "close", false);
    defineNativeMethod(klass, &file_write, "write", false);
    defineNativeMethod(klass, &file_read, "read", false);
    defineNativeMethod(klass, &file_sync, "sync", false);
    defineNativeMethod(klass, &file_flush, "flush", false);
    defineNativeMethod(klass, &file_static_exists_q, "exists?", true);
    defineNativeMethod(klass, &file_static_directory_q, "directory?", true);
    defineNativeMethod(klass, &file_static_file_q, "file?", true);
    defineNativeMethod(klass, &file_static_read_all_lines, "read_all_lines", true);
}