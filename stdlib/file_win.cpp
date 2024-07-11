#include <string>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strsafe.h>

extern "C" {

#include "comet.h"
#include "comet_stdlib.h"
#include "file_common.h"

void file_constructor(void* instanceData)
{
    FileData* data = (FileData*)instanceData;
    data->fp = INVALID_HANDLE_VALUE;
}

void file_destructor(void* data)
{
    FileData* file_data = (FileData*)data;
    if (file_data->fp != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(file_data->fp);
        CloseHandle(file_data->fp);
    }
}


static DWORD translate_flags_to_mode(VALUE flags)
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
    ObjInstance* instance = (ObjInstance*)newInstance(vm, AS_CLASS(klass));
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(instance));
    const char* path = string_get_cstr(arguments[0]);
    DWORD mode = translate_flags_to_mode(arguments[1]);
    DWORD creationDisposition = OPEN_EXISTING;
    if ((mode & GENERIC_WRITE) == GENERIC_WRITE)
    {
        creationDisposition = TRUNCATE_EXISTING;
    }
    DWORD attributes = GetFileAttributesA(string_get_cstr(arguments[0]));
    if (GetFileAttributesA(string_get_cstr(arguments[0])) == INVALID_FILE_ATTRIBUTES && (mode & (GENERIC_READ | GENERIC_WRITE)))
    {
        creationDisposition = CREATE_NEW;
    }
    HANDLE fp = CreateFileA(path, mode, 0, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fp == INVALID_HANDLE_VALUE)
    {
        throw_exception_native(vm, "IOException", "Couldn't open file '%s'", path);
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
    if (data->fp != INVALID_HANDLE_VALUE)
    {

        if (!CloseHandle(data->fp))
        {
            printf("Didn't close the file\n");
        }
        data->fp = INVALID_HANDLE_VALUE;
    }
    return NIL_VAL;
}

VALUE file_write(VM* vm, VALUE self, int UNUSED(arg_count), VALUE* arguments)
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));
    DWORD result = 0;
    const char* buffer = string_get_cstr(arguments[0]);
    WriteFile(data->fp, buffer, (DWORD)strlen(buffer), &result, NULL);
    return create_number(vm, (double)result);
}

VALUE file_write_line(VM *vm, VALUE self, int arg_count, VALUE *arguments)
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));
    DWORD result = 0;
    if (arg_count > 0)
    {
        const char* buffer = string_get_cstr(arguments[0]);
        WriteFile(data->fp, buffer, (DWORD)strlen(buffer), &result, NULL);
    }
    WriteFile(data->fp, "\n", (DWORD)1, NULL, NULL);
    return create_number(vm, (double)result + 1);
}

VALUE file_read(VM* vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));

    LARGE_INTEGER fileSize_struct = { 0 };
    GetFileSizeEx(data->fp, &fileSize_struct);
    LONGLONG fileSize = fileSize_struct.QuadPart;

    char* buffer = ALLOCATE(char, sizeof(char) * (fileSize + 1));
    DWORD bytesRead = 0;
    bool success = ReadFile(data->fp, buffer, fileSize, &bytesRead, NULL);
    if (!success) {
        FILE_NAME_INFO info = {0};
        GetFileInformationByHandleEx(data->fp, FileNameInfo, &info, sizeof(info));
        throw_exception_native(vm, "IOException", "Couldn't read file '%s'\n", info.FileName);
    }
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

VALUE file_sync(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
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
    if (GetFileAttributesA(string_get_cstr(arguments[0])) == INVALID_FILE_ATTRIBUTES)
        return FALSE_VAL;
    return TRUE_VAL;
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
    const char* path = string_get_cstr(arguments[0]);
    HANDLE fp = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (fp == INVALID_HANDLE_VALUE) {
        throw_exception_native(vm, "IOException", "Couldn't open file '%s'\n", path);
        return NIL_VAL;
    }

    LARGE_INTEGER fileSize_struct = {0};
    GetFileSizeEx(fp, &fileSize_struct);
    DWORD fileSize = fileSize_struct.LowPart;

    char* buffer = ALLOCATE(char, sizeof(char) * (fileSize + 1));
    DWORD bytesRead = 0;
    bool success = ReadFile(fp, buffer, fileSize, &bytesRead, NULL);
    CloseHandle(fp);
    if (!success) {
        throw_exception_native(vm, "IOException", "Couldn't read file '%s'\n", path);
    }
    buffer[bytesRead] = '\0';

    VALUE result = list_create(vm);
    push(vm, result);

    size_t index = 0;
    char* current = buffer;
    while (index < bytesRead)
    {
        char* line_end = strchr(current, '\n');
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
        else if (current == line_end)
        {
            VALUE string = copyString(vm, "", 0);
            list_add(vm, result, 1, &string);
            index++;
        }
        else
        {
            break;
        }
        current = line_end + 1;
    }

    FREE_ARRAY(char, buffer, fileSize + 1);

    return pop(vm);
}

VALUE file_static_delete(VM* vm, VALUE klass, int UNUSED(arg_count), VALUE* arguments)
{
    const char* filename = string_get_cstr(arguments[0]);
    bool result = DeleteFileA(filename);
    if (!result)
    {
        throw_exception_native(vm, "IOException", "Could not delete file %s\n", filename);
    }
    return NIL_VAL;
}

VALUE file_static_copy(VM* vm, VALUE klass, int UNUSED(arg_count), VALUE* arguments)
{
    const char* source_filename = string_get_cstr(arguments[0]);
    const char* dest_filename = string_get_cstr(arguments[1]);
    bool result = CopyFileA(source_filename, dest_filename, false);
    if (!result)
    {
        throw_exception_native(vm, "IOException", "Could not copy file %s\n", source_filename);
    }
    return NIL_VAL;
}

VALUE file_read_line(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*args))
{
    std::string output;
    char c = 0;
    DWORD bytesRead = 0;
    FileData* data = GET_NATIVE_INSTANCE_DATA(FileData, OBJ_VAL(self));
    bool success = ReadFile(data->fp, &c, 1, &bytesRead, NULL);
    while (success && bytesRead > 0 && c != '\n')
    {
        output += c;
        success = ReadFile(data->fp, &c, 1, &bytesRead, NULL);
    }
    return copyString(vm, output.c_str(), output.length());
}

}