#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include "liberal.h"
#include "fileBuffer.h"
#include "fileOpener.h"
#include "stringBuffer.h"

/* A good, meaty base-2 chunk of a file, so we don't start reading the thing
 * from disk a character at a time
 */
#define FILE_BUFFER_SIZE 32768

/* Should avoid allocating these on the stack - it could use up memory much too quickly
 * (stacks are usually limited to around 8MB) Which, I suppose is big enough to hold a 
 * decent amount, but don't I want to leave plenty of room for other stuff?
 */
struct FileBuffer
{
   FILE *file;
   const char *filename;
   unsigned short fileNumber;
   unsigned int lineNum;
   unsigned int bufferIndex;
   char buffer[FILE_BUFFER_SIZE + 1];
   size_t charsRead;
   eral_LogicalLine_t currentLine;
};

#if ERAL_DEBUG
void eral_DebugPrintFileBuffer(eral_FileBuffer_t *buffer);
#endif

const char *eral_GetFileBufferFilename(eral_FileBuffer_t *fileBuffer)
{
   return fileBuffer->filename;
}

unsigned int eral_GetFileBufferCurrentLineNo(eral_FileBuffer_t *fileBuffer)
{
   return fileBuffer->lineNum;
}

unsigned short eral_GetFileBufferFileNumber(eral_FileBuffer_t *fileBuffer)
{
   return fileBuffer->fileNumber;
}

eral_FileBuffer_t *eral_CreateFileBuffer(const char *file)
{
   eral_FileBuffer_t *fileBuffer = (eral_FileBuffer_t *)malloc(sizeof(eral_FileBuffer_t));
   fileBuffer->filename = file;
   fileBuffer->file = eral_OpenFile(file, "r", &fileBuffer->fileNumber);
   fileBuffer->lineNum = 0;
   fileBuffer->bufferIndex = 0;
   fileBuffer->charsRead = 0;
   fileBuffer->currentLine.string = NULL;
   fileBuffer->currentLine.length = 0;
   fileBuffer->currentLine.index = 0;
   return fileBuffer;
}

void eral_DeleteFileBuffer(eral_FileBuffer_t *buffer)
{
   ASSERT(buffer->file != NULL);
   fclose(buffer->file);
   if (buffer->currentLine.string != NULL)
   {
      free(buffer->currentLine.string);
   }
   ASSERT(buffer != NULL);
   free(buffer);
}

static void readFileChunk(eral_FileBuffer_t *fileBuffer)
{
   fileBuffer->charsRead = fread(fileBuffer->buffer,
                                 sizeof(*(fileBuffer->buffer)),
                                 FILE_BUFFER_SIZE,
                                 fileBuffer->file);
   if (ferror(fileBuffer->file))
   {
      printf("Unexpected Error (%s) while reading %s\n",
              strerror(errno), fileBuffer->filename);
      exit(1);
   }
   //make life a little easier for ourselves
   fileBuffer->bufferIndex = 0;
}

bool_t eral_FileBufferEOFReached(eral_FileBuffer_t *buffer)
{
   return (bool_t)(feof(buffer->file) && buffer->charsRead == 0);
}

eral_LogicalLine_t *eral_FileBufferGetNextLogicalLine(eral_FileBuffer_t *fileBuffer)
{
   eral_StringBuffer_t *lineBuffer = eral_CreateStringBuffer();
   bool_t lineIsRead = false;
   if (fileBuffer->currentLine.string != NULL)
   {
      free(fileBuffer->currentLine.string);
   }
   while (!lineIsRead)
   {
      if (fileBuffer->bufferIndex == fileBuffer->charsRead)
      {
         readFileChunk(fileBuffer);
         if (eral_FileBufferEOFReached(fileBuffer))
         {
            long lineLength = eral_GetStringBufferLength(lineBuffer);
            if (lineLength > 0)
            {
               fileBuffer->currentLine.length = lineLength;
               fileBuffer->currentLine.string = eral_DestroyBufferNotString(lineBuffer);
               fileBuffer->currentLine.index = 0;
               return &fileBuffer->currentLine;
            }
            eral_DeleteStringBuffer(lineBuffer);
            fileBuffer->currentLine.string = NULL;
            fileBuffer->currentLine.index = 0;
            fileBuffer->currentLine.length = 0;
            return &fileBuffer->currentLine;
         }
      }
      while ((fileBuffer->bufferIndex < fileBuffer->charsRead) && !lineIsRead)
      {
         if (!isBreakingWhiteSpace(fileBuffer->buffer[fileBuffer->bufferIndex]))
         {
            eral_StringBufferAppendChar(lineBuffer,
                                        fileBuffer->buffer[fileBuffer->bufferIndex++]);
         }
         else
         {
            if (fileBuffer->buffer[(fileBuffer->bufferIndex - 1)] == '\\')
            {
               eral_StringBufferUnappendChar(lineBuffer);
            }
            else
            {
               eral_StringBufferAppendChar(lineBuffer, '\0');
               lineIsRead = true;
            }
            //This might seem weird, but the construction here will allow us to handle any
            //of the 3 different line ending types without handling more than a single line
            //in the buffer.
            if (fileBuffer->buffer[fileBuffer->bufferIndex] == '\r')
            {
               fileBuffer->bufferIndex++;
            }
            if (fileBuffer->buffer[fileBuffer->bufferIndex] == '\n')
            {
               fileBuffer->bufferIndex++;
            }
            //We want to increase this regardless of whether we have a full logical line or not,
            //so we keep the line numbers in sync with the source file
            fileBuffer->lineNum++;
         }
      }
   }
   fileBuffer->currentLine.length = eral_GetStringBufferLength(lineBuffer);
   fileBuffer->currentLine.string = eral_DestroyBufferNotString(lineBuffer);
   fileBuffer->currentLine.index = 0;
   return &fileBuffer->currentLine;
}

/**
 * Shift the string left in-place by amountToShift starting at shiftOffset.
 * We don't realloc here, because it's a side effect which would break client
 * referential integrity.
 */
void eral_ShiftLineLeftAndShrink(eral_LogicalLine_t *line,
                                 uint32_t shiftOffset,
                                 int amountToShift)
{
   uint32_t i;
   for (i = 0; i < line->length - (shiftOffset + amountToShift); i++)
   {
      line->string[shiftOffset + i] = line->string[shiftOffset + i + amountToShift];
   }
   line->length -= amountToShift;
   line->string[line->length] = '\0';
}

#if ERAL_DEBUG
void eral_DebugPrintFileBuffer(eral_FileBuffer_t *buffer)
{
   printf("----- FileBuffer ----- \nfilename:\t%s\nlineNum:\t%u\nbufferIndex:\t%u\ncharsRead:\t%" PRIuPTR "\n",
          buffer->filename, buffer->lineNum, buffer->bufferIndex, (uintptr_t)buffer->charsRead);
   printf("Current Char:\t%d\n", buffer->buffer[buffer->bufferIndex]);
}
#endif
