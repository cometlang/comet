#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h> 
#include <stddef.h>  
#include <stdint.h>  

#define NAN_TAGGING (1)
#define DEBUG_PRINT_CODE (1)
#define DEBUG_TRACE_EXECUTION (1)
#define DEBUG_STRESS_GC (1)
#define DEBUG_LOG_GC (1)
#define UINT8_COUNT (UINT8_MAX + 1)

#ifndef UNUSED
#define UNUSED(x) x __attribute__ ((unused))
#endif

#endif 
