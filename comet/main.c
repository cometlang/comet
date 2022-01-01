#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "cometlib.h"
#include "compiler.h"
#include "import.h"

#define stringify(s) #s
#define stringify_value(s) stringify(s)

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

        Value module = compile(&source, &virtualMachine);
        if (module == NIL_VAL)
        {
            runtimeError(&virtualMachine, "compilation error");
        }
        else
        {
            interpret(&virtualMachine, module);
        }
    }
}

static void runFile(const char *path)
{
    Value to_import = copyString(&virtualMachine, path, strlen(path));
    push(&virtualMachine, to_import);
    Value main = import_from_file(&virtualMachine, NULL, to_import);
    InterpretResult result = interpret(&virtualMachine, main);

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
    if (argc >= 2)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "--version", 9) == 0)
            {
                printf("comet programming language, %s\n", stringify_value(VERSION_STRING));
                return 0;
            }
        }
    }
    init_comet(&virtualMachine);

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