#ifndef clox_compiler_h
#define clox_compiler_h

#include "objects.h"
#include "vm.h"

ObjModule* compile(const SourceFile* source, VM *thread);
SourceFile *readSourceFile(const char *path);
void markCompilerRoots(void);

#endif
