
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
RED = 641722192,BLUE = 641722192,GREEN = 641722192,};;
enum class TestFlagBinary : u64 { // enum name
a = 641722192,b = 641722192,c = 641722192,d = 641722192,e = 641722192,};;
enum class TestFlagHex : u64 { // enum name
a = 641722192,b = 641722192,c = 641722192,d = 641722192,e = 641722192,f = 641722192,g = 641722192,h = 641722192,i = 641722192,j = 641722192,k = 641722192,l = 641722192,m = 641722192,n = 641722192,o = 641722192,p = 641722192,q = 641722192,r = 641722192,s = 641722192,t = 641722192,};;
enum class Numbers : s8 { // enum name
NegativeFive = 641722192,NegativeFour = 641722192,NegativeThree = 641722192,NegativeTwo = 641722192,NegativeOne = 641722192,Zero = 641722192,One = 641722192,Two = 641722192,Three = 641722192,Four = 641722192,Five = 641722192,};;
enum class numbers2 : s8 { // enum name
A = 641722192,B = 641722192,C = 641722192,D = 641722192,E = 641722192,};;
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
