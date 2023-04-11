#ifndef BASE_DEFS_H
#define BASE_DEFS_H

#ifdef __cplusplus
extern "C" { 
#endif

#include <stdio.h>

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

typedef i8       b8 ;
typedef i32      b32;

typedef float    f32;
typedef double   f64;

typedef struct {
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 month;
    i32 year;
} datetime;

// I did not end up having multiple graphics backends 
#define AP_OPENGL

#if defined(_WIN32)
#    define AP_EXPORT __declspec(dllexport) 
#elif defined(__EMSCRIPTEN__)
#    include <emscripten/emscripten.h>
#    define AP_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#    define AP_EXPORT 
#endif

#if defined(_WIN32)
#   define AP_IMPORT __declspec(dllimport)
#else
#   define AP_IMPORT
#endif

#define STRINGIFY_NX(a) #a
#define STRINGIFY(a) STRINGIFY_NX(a)

#define CONCAT_NX(a, b) a##b
#define CONCAT(a, b) CONCAT_NX(a, b)

#if _WIN32 
# define BREAK_DEBUGGER() __debugbreak()
#else
# define BREAK_DEBUGGER() (*(volatile int *)0 = 0)
#endif

#ifndef AP_THREAD_VAR
#    if defined(__clang__) || defined(__GNUC__)
#        define AP_THREAD_VAR __thread
#    elif defined(_MSC_VER)
#        define AP_THREAD_VAR __declspec(thread)
#    elif (__STDC_VERSION__ >= 201112L)
#        define AP_THREAD_VAR _Thread_local
#    else
#        error "Invalid compiler/version for thead variable"
#    endif
#endif

#ifdef AP_ASSERT
# define ASSERT(a, b) do { \
    if(!(a)) { \
        fprintf(stderr, "\033[35mAssert Failed: " b "\033[m\n");\
        BREAK_DEBUGGER();\
    } } while(0)
#else
# define ASSERT(a, b)
#endif

#define AP_UNUSED(x) (void)(x)

#define MIN(a, b)     (((a) < (b)) ? (a) : (b))
#define MAX(a, b)     (((a) > (b)) ? (a) : (b))
#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))

#define ALIGN_UP_POW2(x,p)   (((x) + (p) - 1)&~((p) - 1))
#define ALIGN_DOWN_POW2(x,p) ((x)&~((p) - 1))

#define KB(x) ((x) * 1000)
#define MB(x) ((x) * 1000000)
#define GB(x) ((x) * 1000000000)

#define KiB(x) ((x) << 10)
#define MiB(x) ((x) << 20)
#define GiB(x) ((x) << 30) 

#define STATIC_ARR_LEN(arr) ( sizeof(arr) / sizeof(arr[0]) )

#define CREATE_STRUCT(arena, type) \
    (type*)(marena_push(arena, sizeof(type)))
#define CREATE_ZERO_STRUCT(arena, type)   \
    (type*)(marena_push_zero(arena, sizeof(type)))

#define CREATE_ARRAY(arena, type, size) \
    (type*)(marena_push(arena, sizeof(type) * (size)))
#define CREATE_ZERO_ARRAY(arena, type, size)     \
    (type*)(marena_push_zero(arena, sizeof(type) * (size)))

#define SLL_PUSH_FRONT(f, l, n) ((f) == 0 ? \
    ((f) = (l) = (n)) :                     \
    ((n)->next = (f), (f) = (n)))           \

#define SLL_PUSH_BACK(f, l, n) ((f) == 0 ? \
    ((f) = (l) = (n)) :                    \
    ((l)->next = (n), (l) = (n)),          \
    ((n)->next = 0))                       \

#define SLL_POP_FRONT(f, l) ((f) == (l) ? \
    ((f) = (l) = 0) :                     \
    ((f) = (f)->next))                    \

#define DLL_PUSH_BACK(f, l, n) ((f) == 0 ? \
    ((f) = (l) = (n), (n)->next = (n)->prev = 0) :  \
    ((n)->prev = (l), (l)->next = (n), (l) = (n), (n)->next = 0))

#define DLL_PUSH_FRONT(f, l, n) DLL_PUSH_BACK(l, f, n)

#define DLL_REMOVE(f, l, n) ( \
    (f) == (n) ? \
        ((f) == (l) ? \
            ((f) = (l) = (0)) : \
            ((f) = (f)->next, (f)->prev = 0)) : \
        (l) == (n) ? \
            ((l) = (l)->prev, (l)->next = 0) : \
            ((n)->next->prev = (n)->prev, \
            (n)->prev->next = (n)->next))

#ifdef __cplusplus
}
#endif

#endif // BASE_DEFS_H
