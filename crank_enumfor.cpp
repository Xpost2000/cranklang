
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
#include <string>
#include <cstddef>
#include <cstdint>
// the crank array system depends on these two things
#include <vector>
#include <unordered_map>

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

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

typedef u32 uint;
enum class LightSignal : s32 { // enum name
RED = 0,BLUE = 1,GREEN = 2,};;
enum class TestFlagBinary : u64 { // enum name
a = 0,b = 1,c = 2,d = 3,e = 4,};;
enum class TestFlagHex : u64 { // enum name
a = 0,b = 1,c = 2,d = 3,e = 4,f = 5,g = 6,h = 7,i = 8,j = 9,k = 10,l = 11,m = 12,n = 13,o = 14,p = 15,q = 16,r = 17,s = 18,t = 255,};;
enum class Numbers : s8 { // enum name
NegativeFive = -5,NegativeFour = -4,NegativeThree = -3,NegativeTwo = -2,NegativeOne = -1,Zero = 0,One = 1,Two = 2,Three = 3,Four = 4,Five = 5,};;
enum class numbers2 : s8 { // enum name
A = 3,B = 4,C = 5,D = 10,E = 11,};;
extern "C" {
int  printf(char * fmt);}; // end extern "C" 

int  crank_mainpoint_entry(int  argc, std::string  argv[]);
int  crank_mainpoint_entry(int  argc, std::string  argv[])
{
printf("%c\n", "adf"[0]);printf("Here's an interesting for loop");{
int  i = 3;
;int  j = 13;
;while (i<j) {
{
printf("i: %d, j: %d\n", i, j);}
i+=1;j-=1;}
}
printf("I want to get an enum!!! %d %d\n", (s8)numbers2::A, (s8)numbers2::D+(s32)LightSignal::RED);return 0;
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
