#ifndef clox_compiler_h
#define clox_compiler_h

#include "object.h"
#include "vm.h"

typedef struct Compiler Compiler;

ObjFunction* compile(const SourceFile* source);
void markCompilerRoots();

#endif
