
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
enum class LightSignal : i32 { // enum name
RED = 0,BLUE = 1,GREEN = 2,};;
enum class TestFlagBinary : u64 { // enum name
a = 99,b = 99,c = 99,d = 99,e = 99,};;
enum class TestFlagHex : u64 { // enum name
a = 99,b = 99,c = 99,d = 99,e = 99,f = 99,g = 99,h = 99,i = 99,j = 99,k = 99,l = 99,m = 99,n = 99,o = 99,p = 99,q = 99,r = 99,s = 99,t = 99,};;
enum class Numbers : s8 { // enum name
NegativeFive = 99,NegativeFour = 100,NegativeThree = 101,NegativeTwo = 102,NegativeOne = 103,Zero = 104,One = 105,Two = 106,Three = 107,Four = 108,Five = 109,};;
enum class numbers2 : s8 { // enum name
A = 99,B = 100,C = 101,D = 99,E = 100,};;
extern "C" {
int  printf(char * fmt);}; // end extern "C" 

int  crank_mainpoint_entry(int  argc, std::string  argv[]);
int  crank_mainpoint_entry(int  argc, std::string  argv[])
{
printf("Here's an interesting for loop");{
int  i = 3;
;int  j = 13;
;while (i<j) {
{
printf("i: %d, j: %d\n", i, j);}
i+=1;j-=1;}
}
printf("I want to get an enum!!! %d %d\n", (s8)numbers2::A, (s8)numbers2::D+(i32)LightSignal::RED);return 0;
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
