#include <stdio.h>
#include <string.h>

#include "fileBuffer.h"
#include "fileOpener.h"

#define FILE_CONTENTS "\
This is the first line\n\
This is the second line\n\
The third logical line is spread over two physical lines\\\n\
 see - this is the rest of the third logical line\n\
this is the \"fourth\" line and it has whitespace     \\\n\
before the continuation character\n\
The Fifth logical line has a dos line ending\r\n\
No Line Ending at EOF"

enum lines
{
   FIRST_LINE,
   SECOND_LINE,
   THIRD_LINE,
   FOURTH_LINE,
   FIFTH_LINE,
   SIXTH_LINE,
   NUM_LINES
};
const char *lines[NUM_LINES] = {[FIRST_LINE] = "This is the first line",
                                [SECOND_LINE] = "This is the second line",
                                [THIRD_LINE] = "The third logical line is spread over two physical lines see - this is the rest of the third logical line",
                                [FOURTH_LINE] = "this is the \"fourth\" line and it has whitespace     before the continuation character",
                                [FIFTH_LINE] = "The Fifth logical line has a dos line ending",
                                [SIXTH_LINE] = "No Line Ending at EOF"};

#define FILENAME "test_fileBufferHeader.h"

static void SetupFileBufferTest(void)
{
   FILE *file = fopen(FILENAME, "w+");
   fwrite(FILE_CONTENTS, sizeof(char), strlen(FILE_CONTENTS), file);
   fclose(file);
   eral_FileOpenerInitialise();
}

static void TearDownFileBufferTest(void)
{
   remove(FILENAME);
   eral_FileOpenerDelete();
}

static void test_fileBuffer(void)
{
   eral_FileBuffer_t *testBuffer = eral_CreateFileBuffer(FILENAME);
   eral_LogicalLine_t *line = NULL;
   int i = 0;
   while (!eral_FileBufferEOFReached(testBuffer))
   {
      line = eral_FileBufferGetNextLogicalLine(testBuffer);
      printf("Test Line  : '%s'\n", lines[i]);
      printf("Value line : '%s'\n", line->string);
      ASSERT(strncmp(lines[i], line->string, line->length) == 0);
      i++;
   }
   ASSERT(i == NUM_LINES);
   eral_DeleteFileBuffer(testBuffer);
}

int main(void)
{
   SetupFileBufferTest();
   test_fileBuffer();
   TearDownFileBufferTest();
   return 0;
}
