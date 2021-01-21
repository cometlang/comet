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

VALUE file_static_open(VM *vm, VALUE klass, int arg_count, VALUE *arguments)
{
    ObjNativeInstance *instance = (ObjNativeInstance *)newInstance(vm, AS_CLASS(klass));
    if (arg_count != 2)
    {
        fprintf(stderr, "Require 2 arguments a path and opening mode: got %d\n", arg_count);
        // Throw an exception
        return NIL_VAL;
    }
    const char *path = string_get_cstr(arguments[0]);
    const char *mode = string_get_cstr(arguments[1]);
    FILE *fp = fopen(path, mode);
    //should probably check for NULL and free the object, returning NIL, or throwing an exception or something
    ((FileData *)instance->data)->fp = fp;
    return OBJ_VAL(instance);
}

VALUE file_close(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
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

VALUE file_write(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;
    int result = fprintf(data->fp, "%s", string_get_cstr(arguments[0]));
    return create_number(vm, (double)result);
}

VALUE file_read(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;

    fseek(data->fp, 0L, SEEK_END);
    size_t fileSize = ftell(data->fp);
    rewind(data->fp);

    char *buffer = (char *) malloc(sizeof(char) * (fileSize + 1));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, data->fp);
    buffer[bytesRead] = '\0';

    return takeString(vm, buffer, fileSize);
}

VALUE file_flush(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
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

VALUE file_sync(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = AS_NATIVE_INSTANCE(self);
    FileData *data = (FileData *)instance->data;
    int fd = fileno(data->fp);
    fsync(fd);
    return NIL_VAL;
}

VALUE file_static_exists_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(string_get_cstr(arguments[0]), &statbuf) == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE file_static_directory_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(string_get_cstr(arguments[0]), &statbuf) == 0)
    {
        if (statbuf.st_mode & __S_IFDIR)
            return TRUE_VAL;
        return FALSE_VAL;
    }
    return FALSE_VAL;
}

VALUE file_static_file_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    struct stat statbuf;
    if (stat(string_get_cstr(arguments[0]), &statbuf) == 0)
    {
        if (statbuf.st_mode & __S_IFREG)
            return TRUE_VAL;
        return FALSE_VAL;
    }
    return FALSE_VAL;
}

VALUE file_static_read_all_lines(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

void init_file(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "File", &file_constructor, &file_destructor, "Object");
    defineNativeMethod(vm, klass, &file_static_open, "open", true);
    defineNativeMethod(vm, klass, &file_close, "close", false);
    defineNativeMethod(vm, klass, &file_write, "write", false);
    defineNativeMethod(vm, klass, &file_read, "read", false);
    defineNativeMethod(vm, klass, &file_sync, "sync", false);
    defineNativeMethod(vm, klass, &file_flush, "flush", false);
    defineNativeMethod(vm, klass, &file_static_exists_q, "exists?", true);
    defineNativeMethod(vm, klass, &file_static_directory_q, "directory?", true);
    defineNativeMethod(vm, klass, &file_static_file_q, "file?", true);
    defineNativeMethod(vm, klass, &file_static_read_all_lines, "read_all_lines", true);
}