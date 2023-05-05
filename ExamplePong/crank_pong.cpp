
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
struct Color { // struct name
u8  r = 0;
u8  g = 0;
u8  b = 0;
u8  a = 255;
};;
extern "C" {
void  InitWindow(int  width, int  height, char * title);}; // end extern "C" 

extern "C" {
bool  IsKeyPressed(int  key);}; // end extern "C" 

extern "C" {
bool  IsKeyReleased(int  key);}; // end extern "C" 

extern "C" {
bool  IsKeyUp(int  key);}; // end extern "C" 

extern "C" {
bool  IsKeyPressed(int  key);}; // end extern "C" 

extern "C" {
void  CloseWindow();}; // end extern "C" 

extern "C" {
void  BeginDrawing();}; // end extern "C" 

extern "C" {
void  EndDrawing();}; // end extern "C" 

extern "C" {
char * TextFormat(char * text,...);}; // end extern "C" 

extern "C" {
void  DrawText(char * text, int  posX, int  posY, int  fontSize, Color  color);}; // end extern "C" 

enum class Key : i32 { // enum name
KEY_NULL = 0,KEY_APOSTROPHE = 39,KEY_COMMA = 44,KEY_MINUS = 45,KEY_PERIOD = 46,KEY_SLASH = 47,KEY_ZERO = 48,KEY_ONE = 49,KEY_TWO = 50,KEY_THREE = 51,KEY_FOUR = 52,KEY_FIVE = 53,KEY_SIX = 54,KEY_SEVEN = 55,KEY_EIGHT = 56,KEY_NINE = 57,KEY_SEMICOLON = 59,KEY_EQUAL = 61,KEY_A = 65,KEY_B = 66,KEY_C = 67,KEY_D = 68,KEY_E = 69,KEY_F = 70,KEY_G = 71,KEY_H = 72,KEY_I = 73,KEY_J = 74,KEY_K = 75,KEY_L = 76,KEY_M = 77,KEY_N = 78,KEY_O = 79,KEY_P = 80,KEY_Q = 81,KEY_R = 82,KEY_S = 83,KEY_T = 84,KEY_U = 85,KEY_V = 86,KEY_W = 87,KEY_X = 88,KEY_Y = 89,KEY_Z = 90,KEY_LEFT_BRACKET = 91,KEY_BACKSLASH = 92,KEY_RIGHT_BRACKET = 93,KEY_GRAVE = 96,KEY_SPACE = 32,KEY_ESCAPE = 256,KEY_ENTER = 257,KEY_TAB = 258,KEY_BACKSPACE = 259,KEY_INSERT = 260,KEY_DELETE = 261,KEY_RIGHT = 262,KEY_LEFT = 263,KEY_DOWN = 264,KEY_UP = 265,KEY_PAGE_UP = 266,KEY_PAGE_DOWN = 267,KEY_HOME = 268,KEY_END = 269,KEY_CAPS_LOCK = 280,KEY_SCROLL_LOCK = 281,KEY_NUM_LOCK = 282,KEY_PRINT_SCREEN = 283,KEY_PAUSE = 284,KEY_F1 = 290,KEY_F2 = 291,KEY_F3 = 292,KEY_F4 = 293,KEY_F5 = 294,KEY_F6 = 295,KEY_F7 = 296,KEY_F8 = 297,KEY_F9 = 298,KEY_F10 = 299,KEY_F11 = 300,KEY_F12 = 301,KEY_LEFT_SHIFT = 340,KEY_LEFT_CONTROL = 341,KEY_LEFT_ALT = 342,KEY_LEFT_SUPER = 343,KEY_RIGHT_SHIFT = 344,KEY_RIGHT_CONTROL = 345,KEY_RIGHT_ALT = 346,KEY_RIGHT_SUPER = 347,KEY_KB_MENU = 348,KEY_KP_0 = 320,KEY_KP_1 = 321,KEY_KP_2 = 322,KEY_KP_3 = 323,KEY_KP_4 = 324,KEY_KP_5 = 325,KEY_KP_6 = 326,KEY_KP_7 = 327,KEY_KP_8 = 328,KEY_KP_9 = 329,KEY_KP_DECIMAL = 330,KEY_KP_DIVIDE = 331,KEY_KP_MULTIPLY = 332,KEY_KP_SUBTRACT = 333,KEY_KP_ADD = 334,KEY_KP_ENTER = 335,KEY_KP_EQUAL = 336,KEY_BACK = 4,KEY_MENU = 82,KEY_VOLUME_UP = 24,KEY_VOLUME_DOWN = 25,};;
enum class PaddleSide : u8 { // enum name
Left = 0,Right = 1,};;
struct Paddle { // struct name
PaddleSide  side = (u8)PaddleSide::Left;
int  x = 0;
int  y = 0;
int  w = 0;
int  h = 0;
};;
struct GameState { // struct name
Paddle  paddles[2];
int  scores[2];
bool  quit = false;
};;
int  crank_mainpoint_entry(int  argc, std::string  argv[]);
int  crank_mainpoint_entry(int  argc, std::string  argv[])
{
GameState  game_state;
;game_state.scores[0]=0;game_state.scores[1]=0;InitWindow(1024, 768, "Hello Cranky Pong!");while ((!game_state.quit)) 
{
if (IsKeyPressed((i32)Key::KEY_F1)) 


game_state.quit=true;BeginDrawing();{
DrawText(TextFormat("Player 1: %d", game_state.scores[0]), 0, 0, 32, {
255,0,0,255,}
);DrawText(TextFormat("Player 2: %d", game_state.scores[1]), 1024-200, 0, 32, {
0,0,255,255,}
);}
EndDrawing();}

CloseWindow();return 0;
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
