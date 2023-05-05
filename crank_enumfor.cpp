
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
RED = -815797968,BLUE = -815797968,GREEN = -815797968,};;
enum class TestFlagBinary : u64 { // enum name
a = -815797968,b = -815797968,c = -815797968,d = -815797968,e = -815797968,};;
enum class TestFlagHex : u64 { // enum name
a = -815797968,b = -815797968,c = -815797968,d = -815797968,e = -815797968,f = -815797968,g = -815797968,h = -815797968,i = -815797968,j = -815797968,k = -815797968,l = -815797968,m = -815797968,n = -815797968,o = -815797968,p = -815797968,q = -815797968,r = -815797968,s = -815797968,t = -815797968,};;
enum class Numbers : s8 { // enum name
NegativeFive = -815797968,NegativeFour = -815797968,NegativeThree = -815797968,NegativeTwo = -815797968,NegativeOne = -815797968,Zero = -815797968,One = -815797968,Two = -815797968,Three = -815797968,Four = -815797968,Five = -815797968,};;
enum class numbers2 : s8 { // enum name
A = -815797968,B = -815797968,C = -815797968,D = -815797968,E = -815797968,};;
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
printf("i: %d, j: %d\n", i, j, x);}
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
