#ifndef _COMET_IMPORT_H_
#define _COMET_IMPORT_H_

#include "vm.h"

#ifdef __cplusplus
extern "C" {
#endif

    ObjModule *import_from_file(VM *vm, const char *filename, Value import_path);

#ifdef __cplusplus
}
#endif

#endif
