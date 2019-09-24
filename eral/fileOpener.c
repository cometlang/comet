#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "liberal.h"

//A list is probably fine here.  If we need to resolve the filename, it's probably an error condition
typedef struct file_info {
   uint16_t fileno;
   char *filename;
} file_info_t;

static uint16_t current_file_number = 0;
static eral_List_t *fileList = NULL;

void eral_FileOpenerInitialise(void)
{
   fileList = eral_ListCreate();
}

static void DeleteFileInfo(uintptr_t fileInfo)
{
   file_info_t *toDelete = (file_info_t *) fileInfo;
   free(toDelete->filename);
   toDelete->filename = NULL;
   free(toDelete);
}

void eral_FileOpenerDelete(void)
{
   eral_ListDelete(fileList, &DeleteFileInfo);
}

FILE *eral_OpenFile(const char *filename, const char *flags,
                   unsigned short *out_fileno)
{
   FILE *file = NULL;
   file_info_t *newFile = (file_info_t *) malloc(sizeof(file_info_t));
   size_t filename_len = strlen(filename);
   file = fopen(filename, flags);
   if (file == NULL)
   {
      printf("Couldn't open the file '%s'\n", filename);
      exit(1);
   }
   newFile->fileno = current_file_number++;
   *out_fileno = newFile->fileno;
   newFile->filename = (char *) malloc((filename_len + 1) * sizeof(char));
   strncpy(newFile->filename, filename, filename_len + 1);

   eral_ListAppendData(fileList, (uintptr_t) newFile);
   
   return file;
}

const char *eral_ResolveFileNameFromNumber(const uint16_t fileno)
{
   eral_ListIterator_t *iter = NULL;
   file_info_t *temp = NULL;

   if (fileList == NULL)
      return NULL;

   iter = eral_ListGetIterator(fileList);
   temp = (file_info_t *) eral_ListGetNextData(iter);
   while (temp != NULL)
   {
      if (temp->fileno == fileno)
      {
         eral_ListDeleteIterator(iter);
         return temp->filename;
      }
      temp = (file_info_t *) eral_ListGetNextData(iter);
   }

   /* We couldn't find it, so free the iterator and return NULL */
   eral_ListDeleteIterator(iter);
   return NULL;
}
