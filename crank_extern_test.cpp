
/**
 * This preamble is for default types
 * and the headers of the language.
 *
 * Every crank modules needs this, and
 * it's useful for the transpiler.
 *
 * NOTE: this file is copied and pasted into every crank module.
 **/

// obligatory requirements
#include <cstdio>
#include <cmath>
#include <ctime>
#include <string>
#include <cstdlib>
#include <cstdint>

// the crank array system depends on these two things
#include <vector>
#include <unordered_map>

#define assertion(x) assert(x)
#define unimplemented(x) assertion(false && x);
#define bad_case         default: { unimplemented ("no case"); } break
#define invalid_cases()  bad_case
#define array_count(x) (sizeof(x)/sizeof(x[0]))
#define local    static
#define internal static
#define safe_assignment(x) if(x) *x

#define BIT(x)             (1 << x)
#define BIT64(x) (uint64_t)(1LL << x)
#define Toggle_Bit(a, x) (a ^= x)
#define Set_Bit(a, x)    (a |= x)
#define Get_Bit(a, x)    (a & x)
#define STRINGIFY(x) #x

#define Bytes(x)                    (uint64_t)(x)
#define Byte(x)                    (uint64_t)(x)
#define Kilobyte(x)                 (uint64_t)(x * 1024LL)
#define Megabyte(x)                 (uint64_t)(x * 1024LL * 1024LL)
#define Gigabyte(x)                 (uint64_t)(x * 1024LL * 1024LL * 1024LL)
#define Terabyte(x)                 (uint64_t)(x * 1024LL * 1024LL * 1024LL * 1024LL)

#ifndef RELEASE
#define _debugprintf(fmt, args...)   fprintf(stderr, "[%s:%d:%s()]: " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#define _debugprintfhead()   fprintf(stderr, "[%s:%d:%s()]: " ,__FILE__, __LINE__, __func__)
#define _debugprintf1(fmt, args...)  fprintf(stderr,  fmt, ##args)
#define DEBUG_CALL(fn) fn; _debugprintf("calling %s in [%s:%d:%s()]", #fn, __FILE__, __LINE__, __func__)
#else
#define _debugprintf(fmt, args...)  
#define _debugprintfhead(fmt, args...)
#define _debugprintf1(fmt, args...)
#define DEBUG_CALL(_) _;
#endif

#define unused(x) (void)(x)

typedef char* cstring;

typedef float  f32;
typedef double f64;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8; /* byte */

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef u32 uint;
extern "C" {
int  MessageBoxA(void * hwnd, char * lptext, char * caption, uint  type);}; // end extern "C" 

int  crank_mainpoint_entry(int  argc, std::string  arguments[]);
int  crank_mainpoint_entry(int  argc, std::string  arguments[])
{
MessageBoxA(0, "Hello World", "Hello Title", 0);return 0;
}

/**
   This file is copy and pasted wholesale,

   DO NOT TOUCH ME!

   ASSUMED std::string is usable, and vector!
**/

int main(int argc, char** argv) {
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    return crank_mainpoint_entry(arguments.size(), arguments.data());
}
