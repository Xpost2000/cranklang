
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
RED = 1994386224,BLUE = 1994386224,GREEN = 1994386224,};;
enum class TestFlagBinary : u64 { // enum name
a = 1994386224,b = 1994386224,c = 1994386224,d = 1994386224,e = 1994386224,};;
enum class TestFlagHex : u64 { // enum name
a = 1994386224,b = 1994386224,c = 1994386224,d = 1994386224,e = 1994386224,f = 1994386224,g = 1994386224,h = 1994386224,i = 1994386224,j = 1994386224,k = 1994386224,l = 1994386224,m = 1994386224,n = 1994386224,o = 1994386224,p = 1994386224,q = 1994386224,r = 1994386224,s = 1994386224,t = 1994386224,};;
enum class Numbers : s8 { // enum name
NegativeFive = 1994386224,NegativeFour = 1994386224,NegativeThree = 1994386224,NegativeTwo = 1994386224,NegativeOne = 1994386224,Zero = 1994386224,One = 1994386224,Two = 1994386224,Three = 1994386224,Four = 1994386224,Five = 1994386224,};;
enum class numbers2 : s8 { // enum name
A = 1994386224,B = 1994386224,C = 1994386224,D = 1994386224,E = 1994386224,};;
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
