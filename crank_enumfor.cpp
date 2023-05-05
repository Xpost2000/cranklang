
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
RED = 1696589424,BLUE = 1696589424,GREEN = 1696589424,};;
enum class TestFlagBinary : u64 { // enum name
a = 1696589424,b = 1696589424,c = 1696589424,d = 1696589424,e = 1696589424,};;
enum class TestFlagHex : u64 { // enum name
a = 1696589424,b = 1696589424,c = 1696589424,d = 1696589424,e = 1696589424,f = 1696589424,g = 1696589424,h = 1696589424,i = 1696589424,j = 1696589424,k = 1696589424,l = 1696589424,m = 1696589424,n = 1696589424,o = 1696589424,p = 1696589424,q = 1696589424,r = 1696589424,s = 1696589424,t = 1696589424,};;
enum class Numbers : s8 { // enum name
NegativeFive = 1696589424,NegativeFour = 1696589424,NegativeThree = 1696589424,NegativeTwo = 1696589424,NegativeOne = 1696589424,Zero = 1696589424,One = 1696589424,Two = 1696589424,Three = 1696589424,Four = 1696589424,Five = 1696589424,};;
enum class numbers2 : s8 { // enum name
A = 1696589424,B = 1696589424,C = 1696589424,D = 1696589424,E = 1696589424,};;
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
