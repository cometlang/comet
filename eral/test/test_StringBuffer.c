#include <string.h>
#include <stdio.h>

#include "util.h"
#include "stringBuffer.h"

static void test_Allocation(void)
{
	eral_StringBuffer_t *buffer = eral_CreateStringBuffer();
	ASSERT(buffer != NULL);
	ASSERT(eral_StringBufferGetBufferSize(buffer) > 0);
	ASSERT(eral_GetStringBufferLength(buffer) == 0);
	eral_DeleteStringBuffer(buffer);
}

static void test_Append(void)
{
	const char *string = "This is a short string";
	const char *ptr = string;
	eral_StringBuffer_t *buffer = eral_CreateStringBuffer();
	ASSERT(buffer != NULL);
	ASSERT(eral_StringBufferGetBufferSize(buffer) > 0);
	ASSERT(eral_GetStringBufferLength(buffer) == 0);

	do
	{
		eral_StringBufferAppendChar(buffer, *ptr);
	} while (*ptr++);
	eral_StringBufferAppendChar(buffer, '\0');
	eral_PrintStringBuffer(buffer);
	ASSERT(eral_StringBufferStrncmp(buffer, string, eral_GetStringBufferLength(buffer)) == 0);
	ASSERT(eral_GetStringBufferLength(buffer) == strlen(string)); //buffer has an extra for '\0'
	eral_DeleteStringBuffer(buffer);
}

static void test_AppendWithResize(void)
{
	const char *string = "This is an exceedingly long string and it goes on and on my friend.  Some people, started writing it, not knowing what it was, and they'll continue stringing it along if only just because...";
	const char *ptr = string;
	eral_StringBuffer_t *buffer = eral_CreateStringBuffer();
	ASSERT(buffer != NULL);
	ASSERT(eral_StringBufferGetBufferSize(buffer) > 0);
	ASSERT(eral_GetStringBufferLength(buffer) == 0);

	do
	{
		eral_StringBufferAppendChar(buffer, *ptr);
	} while (*ptr++);
	eral_StringBufferAppendChar(buffer, '\0');
	eral_PrintStringBuffer(buffer);
	ASSERT(eral_StringBufferStrncmp(buffer, string, eral_GetStringBufferLength(buffer)) == 0);
	ASSERT(eral_GetStringBufferLength(buffer) == strlen(string));
	eral_DeleteStringBuffer(buffer);
}

int main(void)
{
	test_Allocation();
	test_Append();
	test_AppendWithResize();
	return 0;
}
