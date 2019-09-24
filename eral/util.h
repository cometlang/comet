#ifndef _ERAL_UTIL_H_
#define _ERAL_UTIL_H_

#include <assert.h>
#include "eral_config.h"

#if ERAL_DEBUG
# define ASSERT(x) assert(x)
//Obviously stole the COMPILE_TIME_ASSERT code from:
//http://www.jaggersoft.com/pubs/CVu11_3.html
# define COMPILE_TIME_ASSERT(pred) switch(0){case 0:case pred:;} 
#else
# define ASSERT(x)
#endif

#ifndef UNUSED
#define UNUSED(x) x __attribute__ ((unused))
#endif

#define NULL_DATA ((uintptr_t) 0)

typedef _Bool bool_t;

/* If the two are equal, just return one or the other */
#define max(a, b) ( ((a) > (b)) ? (a) : (b) )
#define min(a, b) ( ((a) < (b)) ? (a) : (b) )


#define isWhiteSpace(c) ((c) == '\t' || (c) == '\v' || \
                         (c) == '\r' || (c) == '\n' || \
                         (c) == '\v' || (c) == ' ')

#define isNonBreakingWhiteSpace(c) ((c) == ' ' || (c) == '\t' || (c) == '\v')

#define isBreakingWhiteSpace(c) ((c) == '\r' || (c) == '\n')

#define isWordChar(c) ( ((c) >= 'A' && (c) <= 'Z') || \
                        ((c) >= 'a' && (c) <= 'z') || \
                        ((c) >= '0' && (c) <= '9') || \
                        ((c) == '_') )

#define isNumber(d) ( ((d) >= '0') && ((d) <= '9') )

#define isOctalChar(d) ( ((d) >= '0') && ((d) <= '7') )

#define isHexChar(x) ( isNumber(x) || (toupper(x) >= 'A' && toupper(x) <= 'F') )

#endif
