#ifndef ERAL_STRING_BUFFER_H
#define ERAL_STRING_BUFFER_H

#include <stdlib.h>
#include "eral_config.h"

typedef struct StringBuffer eral_StringBuffer_t;

eral_StringBuffer_t *eral_CreateStringBuffer(void);

void eral_DeleteStringBuffer(eral_StringBuffer_t *buffer);

char *eral_DestroyBufferNotString(eral_StringBuffer_t *buffer);

void eral_StringBufferAppendChar(eral_StringBuffer_t *buffer, const char c);

void eral_StringBufferAppendString(eral_StringBuffer_t *buffer, const char *string);

void eral_StringBufferUnappendChar(eral_StringBuffer_t *buffer);

unsigned long eral_GetStringBufferLength(eral_StringBuffer_t *buffer);

const char *eral_StringBufferGetString(eral_StringBuffer_t *buffer);

int eral_StringBufferStrncmp(eral_StringBuffer_t *buffer, const char *string, size_t length);

#if ERAL_DEBUG
void eral_PrintStringBuffer(eral_StringBuffer_t *buffer);
unsigned long eral_StringBufferGetBufferSize(eral_StringBuffer_t *buffer);
#endif /* ERAL_DEBUG */

#endif /* ERAL_STRING_BUFFER_H */
