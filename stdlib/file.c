#include "comet.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct fileData
{
    FILE *fp;
} fileData;

void *file_constructor(void)
{
    fileData *data = (fileData *) malloc(sizeof(fileData));
    data->fp = NULL;
    return (void *) data;
}

void file_destructor(void *data)
{
    fileData *file_data = (fileData *) data;
    if (file_data->fp != NULL)
    {
        fflush(file_data->fp);
        fclose(file_data->fp);
    }
    free(file_data);
}

VALUE file_static_open(int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
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

VALUE file_static_exists_q(int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_static_directory_q(int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE file_static_file_q(int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

