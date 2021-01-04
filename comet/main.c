#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl()
{
    SourceFile source;
    char line[1024];
    source.source = line;
    for (;;)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        interpret(&source);
    }
}

static SourceFile *readFile(const char *path)
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

static void runFile(const char *path)
{
    SourceFile *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source->source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char **argv)
{
    initVM();

    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        runFile(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: comet [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}