#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "fileBuffer.h"

void PrettyError(const char *filename, int line_no, int col, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    fprintf(stderr, "%s:%d:%d ", filename, line_no, col);
    fprintf(stderr, format, arguments);
    va_end(arguments);
    exit(EXIT_FAILURE);
}

void eral_FileBufferPrettyError(const eral_FileBuffer_t *buffer, unsigned int index, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    PrettyError(
        eral_GetFileBufferFilename(buffer),
        eral_GetFileBufferCurrentLineNo(buffer),
        index,
        format,
        arguments);
    va_end(arguments);
}
