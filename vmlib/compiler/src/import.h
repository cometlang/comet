#ifndef _COMET_IMPORT_H_
#define _COMET_IMPORT_H_

#include "vm.h"

void import_from_file(VM *vm, const char *filename, const char *path, int path_len);

#endif
