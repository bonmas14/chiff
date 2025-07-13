#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef uint8_t b8;
typedef uint32_t b32;

#define ENSURE_LITERAL_STRING(string) "" string ""

#define UNUSED(x) (void)(x)

#define ERRLOG(...)   fprintf(stderr, __VA_ARGS__)
#define INFLOG(...)   fprintf(stdout, __VA_ARGS__)
#define MEMSET(dest, data, size)    mem_set(dest, data, size)
#define MEMCOPY(dest, source, size) mem_copy(dest, data, size)
#define MEMCOMP(a, b, size)         mem_compare(a, b, size)

#define KB(s) ((u64)(s) * 1024LL)
#define MB(s) (KB(s) * 1024LL)
#define GB(s) (MB(s) * 1024LL)

#define PG(s) ((u64)(s) * KB(4))

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)

#ifdef DEBUG
#define assert(result) { \
    if ((result) == 0) { \
        fprintf(stderr, "\n%s,%zu: --- assertion failed.\n", __FILE__, (u64)__LINE__);\
        __asm__("int3");\
    } \
}
#else 
#define assert(...)
#endif
#include "meow_hash/meow_hash_x64_aesni.h"
#include "memctl.cpp"
#include "allocators.cpp"
#include "list.cpp"
#include "strings.cpp"
