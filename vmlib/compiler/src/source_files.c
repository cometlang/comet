#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comet.h"

SourceFile *readSourceFile(const char *path)
{
    SourceFile *sourcefile = (SourceFile *) malloc(sizeof(SourceFile));
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\"\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    sourcefile->source = (char *)malloc(fileSize + 1);
    if (sourcefile->source == NULL)
    {
        fprintf(stderr, "Could not allocate memory to read \"%s\"\n", path);
        exit(74);
    }

    size_t bytesRead = fread(sourcefile->source, sizeof(char), fileSize, file);
    sourcefile->source[bytesRead] = '\0';

    fclose(file);
    int pathLen = strlen(path);
    sourcefile->path = (char *) malloc(pathLen + 1);
    strncpy(sourcefile->path, path, pathLen + 1);
    return sourcefile;
}