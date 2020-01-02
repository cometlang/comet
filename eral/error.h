#ifndef _COMET_ERAL_ERROR_H_
#define _COMET_ERAL_ERROR_H_

#include "fileBuffer.h"

void PrettyError(const char *filename, int line_no, int col, const char *format, ...);

/**
 * Print a pretty error to stderr (filename:line:col ${error}) using a FileBuffer
 * as the source for the location information.
 *
 * Format and va_args are passed to fprintf(stderr, format, ...);
 *
 * exit(EXIT_FAILURE); is then called - noreturn
 */
void eral_FileBufferPrettyError(const eral_FileBuffer_t *buffer, unsigned int index, const char *format, ...);

#endif
