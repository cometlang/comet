#ifndef _COMET_IMPORT_H_
#define _COMET_IMPORT_H_

#include "vm.h"

#ifdef __cplusplus
extern "C" {
#endif

    Value import_from_file(VM *vm, const char *relative_to_filename, Value to_import);

#ifdef __cplusplus
}
#endif

#endif
