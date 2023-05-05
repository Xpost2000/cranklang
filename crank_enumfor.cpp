
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
RED = -1491080224,BLUE = -1491080224,GREEN = -1491080224,};;
enum class TestFlagBinary : u64 { // enum name
a = -1491080224,b = -1491080224,c = -1491080224,d = -1491080224,e = -1491080224,};;
enum class TestFlagHex : u64 { // enum name
a = -1491080224,b = -1491080224,c = -1491080224,d = -1491080224,e = -1491080224,f = -1491080224,g = -1491080224,h = -1491080224,i = -1491080224,j = -1491080224,k = -1491080224,l = -1491080224,m = -1491080224,n = -1491080224,o = -1491080224,p = -1491080224,q = -1491080224,r = -1491080224,s = -1491080224,t = -1491080224,};;
enum class Numbers : s8 { // enum name
NegativeFive = -1491080224,NegativeFour = -1491080224,NegativeThree = -1491080224,NegativeTwo = -1491080224,NegativeOne = -1491080224,Zero = -1491080224,One = -1491080224,Two = -1491080224,Three = -1491080224,Four = -1491080224,Five = -1491080224,};;
enum class numbers2 : s8 { // enum name
A = -1491080224,B = -1491080224,C = -1491080224,D = -1491080224,E = -1491080224,};;
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
