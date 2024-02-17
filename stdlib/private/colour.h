#ifndef _colour_hpp_
#define _colour_hpp_

#ifdef __cplusplus
extern "C" {
#endif

#include "comet.h"

typedef struct {
    ObjInstance obj;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ColourData_t;

VALUE colour_create(VM *vm, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif


#endif
