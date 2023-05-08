
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
struct AudioStream { // struct name
void * buffer;
void * processor;
uint  sampleRate;
uint  sampleSize;
uint  channels;
};;
struct Sound { // struct name
AudioStream  stream;
uint  frameCount;
};;
extern "C" {
void  PlaySound(Sound  sound);}; // end extern "C" 

extern "C" {
Sound  LoadSound(char * where);}; // end extern "C" 

extern "C" {
void  InitWindow(int  width, int  height, char * title);}; // end extern "C" 

extern "C" {
void  InitAudioDevice();}; // end extern "C" 

extern "C" {
bool  WindowShouldClose();}; // end extern "C" 

extern "C" {
void  SetTargetFPS(int  fps);}; // end extern "C" 

extern "C" {
bool  IsKeyPressed(int  key);}; // end extern "C" 

extern "C" {
bool  IsKeyReleased(int  key);}; // end extern "C" 

extern "C" {
bool  IsKeyUp(int  key);}; // end extern "C" 

extern "C" {
bool  IsKeyDown(int  key);}; // end extern "C" 

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

extern "C" {
int  MeasureText(char * text, int  fontSize);}; // end extern "C" 

extern "C" {
void  DrawRectangle(int  posX, int  posY, int  width, int  height, Color  color);}; // end extern "C" 

extern "C" {
void  ClearBackground(Color  color);}; // end extern "C" 

extern "C" {
f32  GetFrameTime();}; // end extern "C" 

enum class Key : i32 { // enum name
KEY_NULL = 0,KEY_APOSTROPHE = 39,KEY_COMMA = 44,KEY_MINUS = 45,KEY_PERIOD = 46,KEY_SLASH = 47,KEY_ZERO = 48,KEY_ONE = 49,KEY_TWO = 50,KEY_THREE = 51,KEY_FOUR = 52,KEY_FIVE = 53,KEY_SIX = 54,KEY_SEVEN = 55,KEY_EIGHT = 56,KEY_NINE = 57,KEY_SEMICOLON = 59,KEY_EQUAL = 61,KEY_A = 65,KEY_B = 66,KEY_C = 67,KEY_D = 68,KEY_E = 69,KEY_F = 70,KEY_G = 71,KEY_H = 72,KEY_I = 73,KEY_J = 74,KEY_K = 75,KEY_L = 76,KEY_M = 77,KEY_N = 78,KEY_O = 79,KEY_P = 80,KEY_Q = 81,KEY_R = 82,KEY_S = 83,KEY_T = 84,KEY_U = 85,KEY_V = 86,KEY_W = 87,KEY_X = 88,KEY_Y = 89,KEY_Z = 90,KEY_LEFT_BRACKET = 91,KEY_BACKSLASH = 92,KEY_RIGHT_BRACKET = 93,KEY_GRAVE = 96,KEY_SPACE = 32,KEY_ESCAPE = 256,KEY_ENTER = 257,KEY_TAB = 258,KEY_BACKSPACE = 259,KEY_INSERT = 260,KEY_DELETE = 261,KEY_RIGHT = 262,KEY_LEFT = 263,KEY_DOWN = 264,KEY_UP = 265,KEY_PAGE_UP = 266,KEY_PAGE_DOWN = 267,KEY_HOME = 268,KEY_END = 269,KEY_CAPS_LOCK = 280,KEY_SCROLL_LOCK = 281,KEY_NUM_LOCK = 282,KEY_PRINT_SCREEN = 283,KEY_PAUSE = 284,KEY_F1 = 290,KEY_F2 = 291,KEY_F3 = 292,KEY_F4 = 293,KEY_F5 = 294,KEY_F6 = 295,KEY_F7 = 296,KEY_F8 = 297,KEY_F9 = 298,KEY_F10 = 299,KEY_F11 = 300,KEY_F12 = 301,KEY_LEFT_SHIFT = 340,KEY_LEFT_CONTROL = 341,KEY_LEFT_ALT = 342,KEY_LEFT_SUPER = 343,KEY_RIGHT_SHIFT = 344,KEY_RIGHT_CONTROL = 345,KEY_RIGHT_ALT = 346,KEY_RIGHT_SUPER = 347,KEY_KB_MENU = 348,KEY_KP_0 = 320,KEY_KP_1 = 321,KEY_KP_2 = 322,KEY_KP_3 = 323,KEY_KP_4 = 324,KEY_KP_5 = 325,KEY_KP_6 = 326,KEY_KP_7 = 327,KEY_KP_8 = 328,KEY_KP_9 = 329,KEY_KP_DECIMAL = 330,KEY_KP_DIVIDE = 331,KEY_KP_MULTIPLY = 332,KEY_KP_SUBTRACT = 333,KEY_KP_ADD = 334,KEY_KP_ENTER = 335,KEY_KP_EQUAL = 336,KEY_BACK = 4,KEY_MENU = 82,KEY_VOLUME_UP = 24,KEY_VOLUME_DOWN = 25,};;
struct Vec2 { // struct name
f32  x = 0;
f32  y = 0;
};;
Vec2  vec2(f32  x, f32  y);
f32  vec2_length_sq(Vec2  a);
f32  vec2_length(Vec2  a);
Vec2  vec2_normalize(Vec2  a);
extern "C" {
int  printf(char * txt,...);}; // end extern "C" 

extern "C" {
f32  sqrtf(f32  v);}; // end extern "C" 

extern "C" {
u64  time(void * ptr);}; // end extern "C" 

extern "C" {
void  srand(u32  seed);}; // end extern "C" 

extern "C" {
int  rand();}; // end extern "C" 

s32  random_int_ranged(s32  min, s32  max);
enum class PaddleSide : u8 { // enum name
Left = 0,Right = 1,};;
struct Paddle { // struct name
u8  side = (u8)PaddleSide::Left;
f32  x = 0;
f32  y = 0;
f32  w = 0;
f32  h = 0;
};;
int  BALL_MAX_SPEED = 600;
int  BALL_DEFAULT_SPEED = 200;
struct Ball { // struct name
f32  x = 0;
f32  y = 0;
f32  radius = 25;
f32  speed = BALL_DEFAULT_SPEED;
Vec2  direction = {
1,0,}
;
};;
struct GameState { // struct name
Paddle  paddles[2];
Ball  ball;
int  scores[2];
bool  quit = false;
};;
int  DEFAULT_FONT_SIZE = 32;
int  PADDING_X = 22;
int  PADDING_Y = 22;
int  PONG_PADDLE_SPEED = 250;
void  draw_ui(GameState * state);
void  draw_paddle(Paddle * paddle);
bool  paddle_intersect_ball(Paddle * paddle, Ball * ball);
void  draw_ball(Ball * ball);
void  clamp_paddle_position(Paddle * paddle);
void  reset_ball_position(GameState * state);
i32  ball_get_round_winner(Ball * ball);
int  crank_mainpoint_entry(int  argc, std::string  argv[]);
Vec2  vec2(f32  x, f32  y)
{
return {
x: x,y: y,}
;
}
f32  vec2_length_sq(Vec2  a)
{
return ((a.x*a.x)+(a.y*a.y));
}
f32  vec2_length(Vec2  a)
{
return sqrtf(vec2_length_sq(a));
}
Vec2  vec2_normalize(Vec2  a)
{
f32  length = vec2_length(a);
;Vec2  result = {
0,0,}
;
;(result.x=(a.x/length));(result.y=(a.y/length));return result;
}
s32  random_int_ranged(s32  min, s32  max)
{
s32  range = (max-min);
;return ((rand()%range)+min);
}
void  draw_ui(GameState * state)
{
DrawText(TextFormat("Player 1: %d", (*state).scores[0]), PADDING_X, PADDING_Y, DEFAULT_FONT_SIZE, {
255,0,0,255,}
);{
char * text = TextFormat("Player 2: %d", (*state).scores[1]);
;DrawText(text, (1024-(MeasureText(text, DEFAULT_FONT_SIZE)+PADDING_X)), PADDING_Y, DEFAULT_FONT_SIZE, {
0,0,255,255,}
);}
}
void  draw_paddle(Paddle * paddle)
{
Color  color;
;if (((*paddle).side==(u8)PaddleSide::Left)) 
{
(color={
255,0,0,255,}
);}

 else {
(color={
0,0,255,255,}
);}

DrawRectangle((*paddle).x, (*paddle).y, (*paddle).w, (*paddle).h, color);}
bool  paddle_intersect_ball(Paddle * paddle, Ball * ball)
{
return (((((*ball).x<((*paddle).x+(*paddle).w))&&((*paddle).x<((*ball).x+(*ball).radius)))&&((*ball).y<((*paddle).y+(*paddle).h)))&&((*paddle).y<((*ball).y+(*ball).radius)));
}
void  draw_ball(Ball * ball)
{
DrawRectangle((*ball).x, (*ball).y, (*ball).radius, (*ball).radius, {
255,255,255,255,}
);}
void  clamp_paddle_position(Paddle * paddle)
{
if (((*paddle).y<=0)) 
{
((*paddle).y=0);}


if ((((*paddle).y+(*paddle).h)>=768)) 
{
((*paddle).y=(768-(*paddle).h));}


}
void  reset_ball_position(GameState * state)
{
Ball * ball = (&(*state).ball);
;((*ball).x=(512+((*ball).radius/2)));((*ball).y=(384+((*ball).radius/2)));((*ball).speed=BALL_DEFAULT_SPEED);}
i32  ball_get_round_winner(Ball * ball)
{
if (((*ball).x<=0.000000)) 
{
return 1;
}


if ((((*ball).x+(*ball).radius)>=1024)) 
{
return 0;
}


return (-1);
}
int  crank_mainpoint_entry(int  argc, std::string  argv[])
{
GameState  game_state;
;(game_state.scores[0]=0);(game_state.scores[1]=0);(game_state.paddles[0].side=(u8)PaddleSide::Left);(game_state.paddles[0].w=(game_state.paddles[1].w=30));(game_state.paddles[0].h=(game_state.paddles[1].h=60));int  PONG_PADDLE_MARGIN = 45;
;(game_state.paddles[0].x=(0+PONG_PADDLE_MARGIN));(game_state.paddles[1].x=(1024-(game_state.paddles[1].w+PONG_PADDLE_MARGIN)));(game_state.paddles[1].side=(u8)PaddleSide::Right);(game_state.paddles[1].y=(game_state.paddles[0].y=((768/2)-(game_state.paddles[0].h/2))));InitWindow(1024, 768, "Hello Cranky Pong!");InitAudioDevice();SetTargetFPS(60);reset_ball_position((&game_state));srand(time(0));Sound  hit_sound = LoadSound("assets/beep.wav");
;while (((!game_state.quit)&&(!WindowShouldClose()))) 
{
f32  dt = GetFrameTime();
;if (IsKeyPressed((i32)Key::KEY_Q)) 


(game_state.quit=true);BeginDrawing();ClearBackground({
}
);draw_ui((&game_state));draw_paddle((&game_state.paddles[0]));draw_paddle((&game_state.paddles[1]));draw_ball((&game_state.ball));{
Paddle * paddle_target = (&game_state.paddles[0]);
;if (IsKeyDown((i32)Key::KEY_S)) 
{
((*paddle_target).y+=(dt*PONG_PADDLE_SPEED));}


if (IsKeyDown((i32)Key::KEY_W)) 
{
((*paddle_target).y-=(dt*PONG_PADDLE_SPEED));}


}
{
Paddle * paddle_target = (&game_state.paddles[1]);
;if (IsKeyDown((i32)Key::KEY_DOWN)) 
{
((*paddle_target).y+=(dt*PONG_PADDLE_SPEED));}


if (IsKeyDown((i32)Key::KEY_UP)) 
{
((*paddle_target).y-=(dt*PONG_PADDLE_SPEED));}


}
clamp_paddle_position((&game_state.paddles[0]));clamp_paddle_position((&game_state.paddles[1]));{
Ball * ball = (&game_state.ball);
;{
((*ball).x+=((dt*(*ball).speed)*(*ball).direction.x));((*ball).y+=((dt*(*ball).speed)*(*ball).direction.y));}
{
if ((paddle_intersect_ball((&game_state.paddles[0]), ball)||paddle_intersect_ball((&game_state.paddles[1]), ball))) 
{
if (((*ball).speed<=BALL_MAX_SPEED)) 
{
((*ball).speed+=15);}

 else {
((*ball).speed=BALL_MAX_SPEED);}

((*ball).direction.x*=(-1));((*ball).direction.y=(random_int_ranged((-10), 10)/10.000000));((*ball).direction=vec2_normalize((*ball).direction));PlaySound(hit_sound);}


if ((((*ball).y<=0)||(((*ball).y+(*ball).radius)>=768))) 
{
((*ball).direction.y*=(-1));PlaySound(hit_sound);}


}
{
int  scorer = ball_get_round_winner(ball);
;if ((scorer!=(-1))) 
{
reset_ball_position((&game_state));(game_state.scores[scorer]+=1);((*ball).direction.x=random_int_ranged((-1), 1));((*ball).direction.y=(random_int_ranged((-10), 10)/10.000000));((*ball).direction=vec2_normalize((*ball).direction));PlaySound(hit_sound);}


}
}
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
