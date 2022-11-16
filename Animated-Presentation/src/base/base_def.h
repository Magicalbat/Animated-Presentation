#ifndef BASE_DEF_H
#define BASE_DEF_H

#include <stdio.h>
#include <errno.h>

#include <stdint.h>
#include <stdbool.h>

typedef int8_t   i8 ;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;

#if AP_PLATFORM_WINDOWS 
# define BREAK_DEBUGGER() __debugbreak()
#else
# define BREAK_DEBUGGER() (*(volatile int *)0 = 0)
#endif

#ifdef AP_ASSERT
# define ASSERT(a, b) do { if(!(a)) { fprintf(stderr, "%c[35mAssert Failed: %s\n", 0x1b, b); BREAK_DEBUGGER(); } } while(0)
#else
# define ASSERT(a, b)
#endif

#define MIN(a, b)     (((a) < (b)) ? (a) : (b))
#define MAX(a, b)     (((a) > (b)) ? (a) : (b))
#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))

#define ALIGN_UP_POW2(x,p)   (((x) + (p) - 1)&~((p) - 1))
#define ALIGN_DOWN_POW2(x,p) ((x)&~((p) - 1))

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30) 

#endif // BASE_DEF_H