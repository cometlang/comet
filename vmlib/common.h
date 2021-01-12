#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h> 
#include <stddef.h>  
#include <stdint.h>  

#define NAN_TAGGING (1)
#define DEBUG_PRINT_CODE (0)
#define DEBUG_TRACE_EXECUTION (0)
#define DEBUG_STRESS_GC (0)
#define DEBUG_LOG_GC (0)
#define MAX_VAR_COUNT (UINT8_MAX + 1)
#define DEBUG_ASSERT_ENABLED (0)

#ifndef UNUSED
#define UNUSED(x) x __attribute__ ((unused))
#endif

typedef struct {
    char *path;
    char *source;
} SourceFile;


#if DEBUG_ASSERT_ENABLED
#include <assert.h>
# define DEBUG_ASSERT(x) assert(x)
#else
# define DEBUG_ASSERT(x)
#endif

#endif 
