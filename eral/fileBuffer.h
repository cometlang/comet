#ifndef ERAL_FILE_BUFFER_H
#define ERAL_FILE_BUFFER_H

#include <stdio.h>
#include <stdint.h>
#include "util.h"

typedef struct FileBuffer eral_FileBuffer_t;

typedef struct LogicalLine {
   char *string;
   unsigned int index;
   unsigned int length;
} eral_LogicalLine_t;

eral_FileBuffer_t *eral_CreateFileBuffer(const char *file);

void eral_DeleteFileBuffer(eral_FileBuffer_t* buffer);

bool_t eral_FileBufferEOFReached(const eral_FileBuffer_t *buffer);

eral_LogicalLine_t *eral_FileBufferGetNextLogicalLine(eral_FileBuffer_t *fileBuffer);

void eral_ShiftLineLeftAndShrink(eral_LogicalLine_t *line, uint32_t shiftOffset, int amountToShift);

const char *eral_GetFileBufferFilename(const eral_FileBuffer_t *fileBuffer);
unsigned short eral_GetFileBufferFileNumber(const eral_FileBuffer_t *fileBuffer);
unsigned int eral_GetFileBufferCurrentLineNo(const eral_FileBuffer_t *fileBuffer);

#if ERAL_DEBUG
void eral_DebugPrintFileBuffer(eral_FileBuffer_t *buffer);
#endif

#endif /* ERAL_FILE_BUFFER_H */
