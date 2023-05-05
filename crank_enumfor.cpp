
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
RED = -289412080,BLUE = -289412080,GREEN = -289412080,};;
enum class TestFlagBinary : u64 { // enum name
a = -289412080,b = -289412080,c = -289412080,d = -289412080,e = -289412080,};;
enum class TestFlagHex : u64 { // enum name
a = -289412080,b = -289412080,c = -289412080,d = -289412080,e = -289412080,f = -289412080,g = -289412080,h = -289412080,i = -289412080,j = -289412080,k = -289412080,l = -289412080,m = -289412080,n = -289412080,o = -289412080,p = -289412080,q = -289412080,r = -289412080,s = -289412080,t = -289412080,};;
enum class Numbers : s8 { // enum name
NegativeFive = -289412080,NegativeFour = -289412080,NegativeThree = -289412080,NegativeTwo = -289412080,NegativeOne = -289412080,Zero = -289412080,One = -289412080,Two = -289412080,Three = -289412080,Four = -289412080,Five = -289412080,};;
enum class numbers2 : s8 { // enum name
A = -289412080,B = -289412080,C = -289412080,D = -289412080,E = -289412080,};;
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
