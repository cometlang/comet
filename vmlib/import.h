#ifndef _COMET_IMPORT_H_
#define _COMET_IMPORT_H_

#include "vm.h"

ObjFunction *import_from_file(VM *vm, const char *filename, Value import_path);

#endif
