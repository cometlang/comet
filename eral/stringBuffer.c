#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util.h>

#include "stringBuffer.h"

#define MCC_INITIAL_STRING_BUFFER_LENGTH 30

struct StringBuffer {
   char *string;
   unsigned long stringLength;
   unsigned long bufferSize;
};


eral_StringBuffer_t *eral_CreateStringBuffer()
{
   eral_StringBuffer_t *result = (eral_StringBuffer_t *) malloc(sizeof(eral_StringBuffer_t));
   ASSERT(result != NULL);
   result->string = (char *) calloc(sizeof(char), MCC_INITIAL_STRING_BUFFER_LENGTH);
   ASSERT(result->string != NULL);
   result->bufferSize = MCC_INITIAL_STRING_BUFFER_LENGTH;
   result->stringLength = 0;
   return result;
}

void eral_DeleteStringBuffer(eral_StringBuffer_t *buffer)
{
   ASSERT(buffer != NULL);
   free(buffer->string);
   free(buffer);
}

char *eral_DestroyBufferNotString(eral_StringBuffer_t *buffer)
{
   //+1 leaves space for the NUL
   char *string = (char *) realloc(buffer->string,
                                   buffer->stringLength+1);
   if (buffer->stringLength == 0)
   {
      free(string);
      string = NULL;
   }
   ASSERT(buffer != NULL);
   free(buffer);
   return string;
}

const char *eral_StringBufferGetString(eral_StringBuffer_t *buffer)
{
   return (const char *) buffer->string;
}

void eral_StringBufferAppendChar(eral_StringBuffer_t *buffer, const char c)
{
   if (buffer->stringLength == buffer->bufferSize)
   {
      buffer->bufferSize *= 2;
      buffer->string = (char *) realloc(buffer->string, buffer->bufferSize);
      /* Is an assert here good enough, or should I error? */
      /* or should I wrap the mem functions with a function that errors clearly when we run out of memory? */
      ASSERT(buffer->string != NULL);
   }
   buffer->string[buffer->stringLength] = c;
   if (c != '\0')
      buffer->stringLength++;
}

void eral_StringBufferAppendString(eral_StringBuffer_t *buffer, const char *string)
{
   int len = strlen(string);
   int i;
   for (i = 0; i < len; i++)
   {
      eral_StringBufferAppendChar(buffer, string[i]);
   }
}

void eral_StringBufferUnappendChar(eral_StringBuffer_t *buffer)
{
   buffer->stringLength--;
}

unsigned long eral_GetStringBufferLength(eral_StringBuffer_t *buffer)
{
   return buffer->stringLength;
}

int eral_StringBufferStrncmp(eral_StringBuffer_t *buffer, const char *string, size_t length)
{
   return strncmp((const char *)buffer->string, string, length);
}

#if ERAL_DEBUG
unsigned long eral_StringBufferGetBufferSize(eral_StringBuffer_t *buffer)
{
   return buffer->bufferSize;
}

void eral_PrintStringBuffer(eral_StringBuffer_t *buffer)
{
   printf("BufferSize:\t%ld\nString Length:\t%ld\nString:\t'%s'\n",
          buffer->bufferSize, buffer->stringLength, buffer->string);
}
#endif /* ERAL_DEBUG */
