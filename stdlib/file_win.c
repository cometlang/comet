#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "comet.h"
#include "comet_stdlib.h"
#include "file_common.h"

typedef struct fileData
{
    ObjNativeInstance obj;
    HANDLE fp;
    bool is_binary;
} FileData;

void file_constructor(void *instanceData)
{
    FileData* data = (FileData *) instanceData;
    data->fp = NULL;
}

void file_destructor(void* data)
{
    FileData* file_data = (FileData*)data;
    if (file_data->fp != NULL)
    {
        fflush(file_data->fp);
        fclose(file_data->fp);
    }
}

static uint64_t translate_flags_to_mode(VALUE flags)
{
    uint64_t flag_value = enumvalue_get_value(flags);
    if (flag_value & FOPEN_READ_ONLY)
    {
        return GENERIC_READ;
    }
    else if (flag_value & FOPEN_READ_WRITE)
    {
        return GENERIC_READ | GENERIC_WRITE;
    }
    else if (flag_value & FOPEN_APPEND)
    {
        return FILE_APPEND_DATA;
    }
    return GENERIC_READ;
}


VALUE file_static_open(VM* vm, VALUE klass, int UNUSED(arg_count), VALUE* arguments)
{
    ObjNativeInstance* instance = (ObjNativeInstance*)newInstance(vm, AS_CLASS(klass));
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(instance));
    const char* path = string_get_cstr(arguments[0]);
    uint64_t mode = translate_flags_to_mode(arguments[1]);
    HANDLE fp = CreateFileW(path, mode, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fp == INVALID_HANDLE_VALUE)
    {
        // GC will take care of the instance
        throw_exception_native(vm, "IOException", strerror(errno));
        return NIL_VAL;
    }
    uint64_t flag_value = enumvalue_get_value(arguments[1]);
    data->fp = fp;
    data->is_binary = (bool)(flag_value & FOPEN_BINARY);
    return OBJ_VAL(instance);
}

VALUE file_close(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));
    if (data->fp != NULL)
    {
        CloseHandle(data->fp);
        data->fp = NULL;
    }
    return NIL_VAL;
}

VALUE file_write(VM* vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));
    int result = 0;
    const char* buffer = string_get_cstr(arguments[0]);
    WriteFile(data->fp, buffer, (DWORD)strlen(buffer), &result, NULL);
    return create_number(vm, (double)result);
}

VALUE file_read(VM* vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));

    DWORD fileSize = 0;
    GetFileSize(data->fp, &fileSize);

    char* buffer = ALLOCATE(char, sizeof(char) * (fileSize + 1));
    DWORD bytesRead = 0;
    ReadFile(data->fp, buffer, fileSize, &bytesRead, NULL);
    buffer[bytesRead] = '\0';

    // If data->is_binary, create a ByteSequence, otherwise takeString
    return takeString(vm, buffer, bytesRead + 1);
}

VALUE file_flush(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));
    bool result = FlushFileBuffers(data->fp);
    if (!result)
    {
        throw_exception_native(vm, "IOException", "Unable to flush file");
    }
    return NIL_VAL;
}

VALUE file_static_exists_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE* arguments)
{
    struct stat statbuf;
    if (stat(string_get_cstr(arguments[0]), &statbuf) == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE file_static_directory_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE* arguments)
{
    struct stat statbuf;
    if (stat(string_get_cstr(arguments[0]), &statbuf) == 0)
    {
        if (statbuf.st_mode & _S_IFDIR)
            return TRUE_VAL;
        return FALSE_VAL;
    }
    return FALSE_VAL;
}

VALUE file_static_file_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE* arguments)
{
    struct stat statbuf;
    if (stat(string_get_cstr(arguments[0]), &statbuf) == 0)
    {
        if (statbuf.st_mode & _S_IFREG)
            return TRUE_VAL;
        return FALSE_VAL;
    }
    return FALSE_VAL;
}

VALUE file_static_read_all_lines(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FILE* fp = fopen(string_get_cstr(arguments[0]), string_get_cstr(arguments[1]));

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    rewind(fp);

    char* buffer = ALLOCATE(char, sizeof(char) * (fileSize + 1));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, fp);
    buffer[bytesRead] = '\0';

    VALUE result = list_create(vm);
    push(vm, result);

    uint32_t index = 0;
    char* current = buffer;
    while (index < fileSize)
    {
        char* line_end = strchr(current, '\n');
        int length = line_end - current;
        VALUE string = copyString(vm, current, length);
        list_add(vm, result, 1, &string);
        index += length;
        current = line_end + 1;
    }

    FREE_ARRAY(char, buffer, fileSize + 1);

    return pop(vm);
}

void init_file(VM* vm)
{
    VALUE klass = defineNativeClass(vm, "File", &file_constructor, &file_destructor, "Object", CLS_FILE, sizeof(FileData), false);
    defineNativeMethod(vm, klass, &file_static_open, "open", 2, true);
    defineNativeMethod(vm, klass, &file_close, "close", 0, false);
    defineNativeMethod(vm, klass, &file_write, "write", 1, false);
    defineNativeMethod(vm, klass, &file_read, "read", 0, false);
    defineNativeMethod(vm, klass, &file_flush, "sync", 0, false); // Windows doesn't really have the "sync" concept, so do our best.
    defineNativeMethod(vm, klass, &file_flush, "flush", 0, false);
    defineNativeMethod(vm, klass, &file_static_exists_q, "exists?", 1, true);
    defineNativeMethod(vm, klass, &file_static_directory_q, "directory?", 1, true);
    defineNativeMethod(vm, klass, &file_static_file_q, "file?", 1, true);
    defineNativeMethod(vm, klass, &file_static_read_all_lines, "read_all_lines", 1, true);
}
