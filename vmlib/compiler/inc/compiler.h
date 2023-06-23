#ifndef clox_compiler_h
#define clox_compiler_h

#include "objects.h"
#include "vm.h"

#define NEW_HASH_PARAM_VALUE ((uint16_t) 0x8000)
#define NEW_LIST_PARAM_VALUE ((uint16_t) 0x4000)

Value compile(const SourceFile* source, VM *thread);
SourceFile *readSourceFile(const char *path);
void freeSourceFile(SourceFile *source);

#endif
