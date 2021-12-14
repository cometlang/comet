#include "comet.h"
#include "comet_stdlib.h"
#include "file_common.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct fileData
{
    ObjNativeInstance obj;
    FILE *fp;
    VALUE open_flags;
} FileData;

void file_constructor(void *instanceData)
{
    FileData *data = (FileData *)instanceData;
    data->fp = NULL;
}

void file_destructor(void *data)
{
    FileData *file_data = (FileData *)data;
    if (file_data->fp != NULL)
    {
        fflush(file_data->fp);
        fclose(file_data->fp);
    }
}

static const char* translate_flags_to_mode(VALUE flags)
{
    uint64_t flag_value = enumvalue_get_value(flags);
    bool include_bin = (flag_value & FOPEN_BINARY);
    if (flag_value & FOPEN_READ_ONLY)
    {
        return include_bin ? "rb" : "r";
    }
    else if (flag_value & FOPEN_READ_WRITE)
    {
        return include_bin ? "wb+" : "w+";
    }
    else if (flag_value & FOPEN_APPEND)
    {
        return include_bin ? "ab+" : "a+";
    }
    return "r";
}

VALUE file_static_open(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE *arguments)
{
    ObjNativeInstance *instance = (ObjNativeInstance *)newInstance(vm, AS_CLASS(klass));
    const char *path = string_get_cstr(arguments[0]);
    const char *mode = translate_flags_to_mode(arguments[1]);
    FILE *fp = fopen(path, mode);
    if (fp == NULL)
    {
        // GC will take care of the instance
        throw_exception_native(vm, "IOException", strerror(errno));
        return NIL_VAL;
    }
    FileData *data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(instance));
    data->fp = fp;
    data->open_flags = arguments[1];

    return OBJ_VAL(instance);
}

VALUE file_close(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData *data = GET_NATIVE_INSTANCE_DATA(FileData, self);
    if (data->fp != NULL)
    {
        fclose(data->fp);
        data->fp = NULL;
    }
    return NIL_VAL;
}

VALUE file_write(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    FileData *data = GET_NATIVE_INSTANCE_DATA(FileData, self);
    int result = fprintf(data->fp, "%s", string_get_cstr(arguments[0]));
    return create_number(vm, (double)result);
}

VALUE file_read(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData *data = GET_NATIVE_INSTANCE_DATA(FileData, self);

    fseek(data->fp, 0L, SEEK_END);
    size_t fileSize = ftell(data->fp);
    rewind(data->fp);

    char *buffer = ALLOCATE(char, sizeof(char) * (fileSize + 1));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, data->fp);
    buffer[bytesRead] = '\0';

    return takeString(vm, buffer, fileSize);
}

VALUE file_flush(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData *data = GET_NATIVE_INSTANCE_DATA(FileData, self);
    int result = fflush(data->fp);
    if (result == EOF)
    {
        fprintf(stderr, "%s", strerror(errno));
    }
    return NIL_VAL;
}

VALUE file_sync(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData *data = GET_NATIVE_INSTANCE_DATA(FileData, self);
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

VALUE file_static_read_all_lines(VM *vm, VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    FILE *fp = fopen(string_get_cstr(arguments[0]), "r");
    if (fp == NULL) {
        throw_exception_native(vm, "IOException", strerror(errno));
    }

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    rewind(fp);

    char *buffer = ALLOCATE(char, sizeof(char) * (fileSize + 1));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, fp);
    buffer[bytesRead] = '\0';

    VALUE result = list_create(vm);
    push(vm, result);

    size_t index = 0;
    char *current = buffer;
    while (index < fileSize)
    {
        char *line_end = strchr(current, '\n');
        if (line_end == NULL)
        {
            line_end = buffer + fileSize;
        }
        if (line_end > current)
        {
            size_t length = line_end - current;
            VALUE string = copyString(vm, current, length);
            list_add(vm, result, 1, &string);
            index += length;
        }
        else
        {
            index++;
        }
        current = line_end + 1;
    }

    FREE_ARRAY(char, buffer, fileSize + 1);

    return pop(vm);
}

VALUE file_static_delete(VM *vm, VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    int result = unlink(string_get_cstr(arguments[0]));
    if (result != 0)
    {
        runtimeError(vm, "Could not delete file: %s\n", strerror(errno));
    }
    return NIL_VAL;
}

void init_file(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "File", &file_constructor, &file_destructor, "Object", CLS_FILE, sizeof(FileData), false);
    defineNativeMethod(vm, klass, &file_static_open, "open", 2, true);
    defineNativeMethod(vm, klass, &file_close, "close", 0, false);
    defineNativeMethod(vm, klass, &file_write, "write", 1, false);
    defineNativeMethod(vm, klass, &file_read, "read", 0, false);
    defineNativeMethod(vm, klass, &file_sync, "sync", 0, false);
    defineNativeMethod(vm, klass, &file_flush, "flush", 0, false);
    defineNativeMethod(vm, klass, &file_static_exists_q, "exists?", 1, true);
    defineNativeMethod(vm, klass, &file_static_directory_q, "directory?", 1, true);
    defineNativeMethod(vm, klass, &file_static_file_q, "file?", 1, true);
    defineNativeMethod(vm, klass, &file_static_read_all_lines, "read_all_lines", 1, true);
    defineNativeMethod(vm, klass, &file_static_delete, "delete", 1, true);
}