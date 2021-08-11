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

void initArgv(VM *vm, int argc, const char **argv)
{
    VALUE argv_list = list_create(vm);
    push(vm, argv_list);
    for (int i = 2; i < argc; i++)
    {
        VALUE arg = copyString(vm, argv[i], strlen(argv[i]));
        push(vm, arg);
        list_add(vm, argv_list, 1, &arg);
        pop(vm);
    }
    addGlobal(copyString(vm, "ARGV", 4), argv_list);
    pop(vm);
}

int main(int argc, const char **argv)
{
    initGlobals();
    initVM(&virtualMachine);
    init_stdlib(&virtualMachine);
    common_strings[STRING_INIT] = copyString(&virtualMachine, "init", 4);
    common_strings[STRING_MOD_INIT_FUNC_NAME] = copyString(&virtualMachine, "__init", 6);
    common_strings[STRING_HASH] = copyString(&virtualMachine, "hash", 4);
    common_strings[STRING_TO_STRING] = copyString(&virtualMachine, "to_string", 9);

    initArgv(&virtualMachine, argc, argv);

    if (argc == 1)
    {
        repl();
    }
    else if (argc >= 2)
    {
        runFile(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: comet [path]\n");
        exit(64);
    }

#ifdef WIN32
    socket_cleanup();
#endif
    freeVM(&virtualMachine);
    finalizeGarbageCollection();
    freeGlobals();
    return 0;
}