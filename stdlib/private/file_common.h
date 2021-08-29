#ifndef COMET_FILE_COMMON_H_
#define COMET_FILE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "comet.h"

extern VALUE fopen_params;

#define FOPEN_READ_ONLY 1
#define FOPEN_READ_WRITE 2
#define FOPEN_APPEND 4
#define FOPEN_BINARY 8

#ifdef __cplusplus
}
#endif

#endif
