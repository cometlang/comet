#ifndef _ERAL_FILE_OPENER_H_
#define _ERAL_FILE_OPENER_H_

#include <stdio.h>
#include <stdint.h>

void eral_FileOpenerInitialise(void);

void eral_FileOpenerDelete(void);

FILE *eral_OpenFile(const char *filename, const char *flags,
                   unsigned short *out_fileno);

const char *eral_ResolveFileNameFromNumber(const uint16_t fileno);
#endif
