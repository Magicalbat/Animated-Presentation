#ifndef BASE_DEF_H
#define BASE_DEF_H

#include <stdint.h>
#include <stdbool.h>

#if AP_PLATFORM_WINDOWS 
#define BREAK_DEBUGGER() __debugbreak()
#else
#define BREAK_DEBUGGER() (*(volatile int *)0 = 0)
#endif

#ifdef AP_ASSERT
#define ASSERT(a) do { if(!(a)) { BREAK_DEBUGGER(); } } while(0)
#else
#define ASSERT(a)
#endif

#define MIN(a, b)     (((a) < (b)) ? (a) : (b))
#define MAX(a, b)     (((a) > (b)) ? (a) : (b))
#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30) 

#endif