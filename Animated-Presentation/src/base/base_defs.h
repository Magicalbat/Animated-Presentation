#ifndef BASE_DEFS_H
#define BASE_DEFS_H

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <inttypes.h>
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

typedef u8       b8;

typedef float    f32;
typedef double   f64;

#define STRINGIFY_NX(a) #a
#define STRINGIFY(a) STRINGIFY_NX(a)

#define CONCAT_NX(a, b) a##b
#define CONCAT(a, b) CONCAT_NX(a, b)

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

#define CREATE_STRUCT(arena, type) \
    (type*)(arena_alloc(arena, sizeof(type)))
#define CREATE_ZERO_STRUCT(arena, var, type)   \
	(type*)(arena_alloc(arena, sizeof(type))); \
	memset(var, 0, sizeof(type))

#define CREATE_ARRAY(arena, type, size) \
    (type*)(arena_alloc(arena, sizeof(type) * (size)))
#define CREATE_ZERO_ARRAY(arena, var, type, size)     \
    (type*)(arena_alloc(arena, sizeof(type) * (size))); \
    memset(var, 0, sizeof(type) * size)

#define FOR_SLL(type, f, var)    for(type* var = f; var != NULL; var=var->next)

#define SLL_PUSH_FRONT(f, l, n) ( (f) == 0 ? \
    ((f) = (l) = (n)) :                      \
    ((n)->next = (f), (f) = (n))             \
)
#define SLL_PUSH_BACK(f, l, n) ( (f) == 0 ? \
    ((f) = (l) = (n)) :                     \
    ((l)->next = (n), (l) = (n)),           \
    ((n)->next = 0)                         \
)
#define SLL_POP_FRONT(f, l) ( (f) == (l) ? \
    ((f) = (l) = 0) :                      \
    ((f) = (f)->next)                      \
)

#define TIME_REGION(t) for(struct { u64 start; u64 end; } _i_ = { os_now_microseconds(), 0 }; !_i_.end; _i_.end = os_now_microseconds(), t = _i_.end - _i_.start)

#endif // BASE_DEFS_H