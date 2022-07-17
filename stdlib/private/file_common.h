#ifndef COMET_FILE_COMMON_H_
#define COMET_FILE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <Windows.h>
#else
#include <stdio.h>
#endif
#include "comet.h"

extern VALUE fopen_params;

#define FOPEN_READ_ONLY 1
#define FOPEN_READ_WRITE 2
#define FOPEN_APPEND 4
#define FOPEN_BINARY 8


typedef struct fileData
{
    ObjInstance obj;
#ifdef WIN32
    HANDLE fp;
    bool is_binary;
#else
    FILE* fp;
    VALUE open_flags;
#endif
} FileData;


void file_constructor(void* instanceData);
void file_destructor(void* data);

VALUE file_static_open(VM* vm, VALUE klass, int arg_count, VALUE *arguments);
VALUE file_static_exists_q(VM* vm, VALUE klass, int arg_count, VALUE* arguments);
VALUE file_static_directory_q(VM* vm, VALUE klass, int arg_count, VALUE* arguments);
VALUE file_static_file_q(VM* vm, VALUE klass, int arg_count, VALUE* arguments);
VALUE file_static_read_all_lines(VM* vm, VALUE klass, int arg_count, VALUE* arguments);
VALUE file_static_delete(VM* vm, VALUE klass, int arg_count, VALUE* arguments);
VALUE file_static_rename(VM* vm, VALUE klass, int arg_count, VALUE* arguments);
VALUE file_static_copy(VM* vm, VALUE klass, int arg_count, VALUE* arguments);

VALUE file_close(VM* vm, VALUE self, int arg_count, VALUE* arguments);
VALUE file_write(VM* vm, VALUE self, int arg_count, VALUE* arguments);
VALUE file_read(VM* vm, VALUE self, int arg_count, VALUE* arguments);
VALUE file_sync(VM* vm, VALUE self, int arg_count, VALUE* arguments);
VALUE file_flush(VM* vm, VALUE self, int arg_count, VALUE* arguments);


#ifdef __cplusplus
}
#endif

#endif
