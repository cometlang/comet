#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "cometlib.h"
#include "compiler.h"

static VM virtualMachine;

static void repl()
{
    SourceFile source;
    char line[1024];
    source.source = line;
    source.path = "";
    for (;;)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        interpret(&virtualMachine, &source);
    }
}

static void runFile(const char *path)
{
    SourceFile *source = readSourceFile(path);
    InterpretResult result = interpret(&virtualMachine, source);
    free(source->path);
    free(source->source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char **argv)
{
    initGlobals();
    initVM(&virtualMachine);
    init_stdlib(&virtualMachine);
    common_strings[STRING_INIT] = copyString(&virtualMachine, "init", 4);
    common_strings[STRING_HASH] = copyString(&virtualMachine, "hash", 4);
    common_strings[STRING_TO_STRING] = copyString(&virtualMachine, "to_string", 9);

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

    freeVM(&virtualMachine);
    finalizeGarbageCollection();
    freeGlobals();
    return 0;
}