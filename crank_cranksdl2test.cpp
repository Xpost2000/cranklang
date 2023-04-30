
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
struct Common_Event { // struct name
u32  type;
u32  timestamp;
};;
struct Window_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u8  event;
u8  padding1;
u8  padding2;
u8  padding3;
i32  data1;
i32  data2;
};;
struct Keysym { // struct name
i32  scancode;
i32  sym;
u16  mod;
u32  unused;
};;
struct Keyboard_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u8  state;
u8  repeat;
u8  padding2;
u8  padding3;
Keysym  keysym;
};;
struct Text_Editing_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u8  text[32];
i32  start;
i32  length;
};;
struct Text_Input_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u8  text[32];
};;
struct Mouse_Motion_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u32  which;
u32  state;
i32  x;
i32  y;
i32  xrel;
i32  yrel;
};;
struct Mouse_Button_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u32  which;
u8  button;
u8  state;
u8  clicks;
u8  padding1;
i32  x;
i32  y;
};;
struct Mouse_Wheel_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
u32  which;
i32  x;
i32  y;
u32  direction;
};;
struct Joy_Axis_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
u8  axis;
u8  padding1;
u8  padding2;
u8  padding3;
i16  value;
u16  padding4;
};;
struct Joy_Ball_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
u8  ball;
u8  padding1;
u8  padding2;
u8  padding3;
i16  xrel;
i16  yrel;
};;
struct Joy_Hat_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
u8  hat;
u8  value;
u8  padding1;
u8  padding2;
};;
struct Joy_Button_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
u8  button;
u8  state;
u8  padding1;
u8  padding2;
};;
struct Joy_Device_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
};;
struct Controller_Axis_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
u8  axis;
u8  padding1;
u8  padding2;
u8  padding3;
i16  value;
u16  padding4;
};;
struct Controller_Button_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
u8  button;
u8  state;
u8  padding1;
u8  padding2;
};;
struct Controller_Device_Event { // struct name
u32  type;
u32  timestamp;
i32  which;
};;
struct Audio_Device_Event { // struct name
u32  type;
u32  timestamp;
u32  which;
u8  iscapture;
u8  padding1;
u8  padding2;
u8  padding3;
};;
struct Touch_Finger_Event { // struct name
u32  type;
u32  timestamp;
i64  touch_id;
i64  finger_id;
f32  x;
f32  y;
f32  dx;
f32  dy;
f32  pressure;
};;
struct Multi_Gesture_Event { // struct name
u32  type;
u32  timestamp;
i64  touch_id;
f32  d_theta;
f32  d_dist;
f32  x;
f32  y;
u16  num_fingers;
u16  padding;
};;
struct Dollar_Gesture_Event { // struct name
u32  type;
u32  timestamp;
i64  touch_id;
i64  gesture_id;
u32  num_fingers;
f32  error;
f32  x;
f32  y;
};;
struct Drop_Event { // struct name
u32  type;
u32  timestamp;
char * file;
u32  window_id;
};;
struct Quit_Event { // struct name
u32  type;
u32  timestamp;
};;
struct OS_Event { // struct name
u32  type;
u32  timestamp;
};;
struct User_Event { // struct name
u32  type;
u32  timestamp;
u32  window_id;
i32  code;
void * data1;
void * data2;
};;
struct Sys_Wm_Event { // struct name
u32  type;
u32  timestamp;
void * msg;
};;
union Event { // union name
u32  type;
Common_Event  common;
Window_Event  window;
Keyboard_Event  key;
Text_Editing_Event  edit;
Text_Input_Event  text;
Mouse_Motion_Event  motion;
Mouse_Button_Event  button;
Mouse_Wheel_Event  wheel;
Joy_Axis_Event  jaxis;
Joy_Ball_Event  jball;
Joy_Hat_Event  jhat;
Joy_Button_Event  jbutton;
Joy_Device_Event  jdevice;
Controller_Axis_Event  caxis;
Controller_Button_Event  cbutton;
Controller_Device_Event  cdevice;
Audio_Device_Event  adevice;
Quit_Event  quit;
User_Event  user;
Sys_Wm_Event  syswm;
Touch_Finger_Event  tfinger;
Multi_Gesture_Event  mgesture;
Dollar_Gesture_Event  dgesture;
Drop_Event  drop;
u8  padding[56];
};;
extern "C" {
i32  SDL_Init(u32  flags);}; // end extern "C" 

extern "C" {
void  SDL_Quit();}; // end extern "C" 

extern "C" {
void * SDL_CreateWindow(char * title, i32  x, i32  y, i32  w, i32  h, u32  flags);}; // end extern "C" 

extern "C" {
i32  SDL_PollEvent(Event * event);}; // end extern "C" 

extern "C" {
int  printf(char * format);}; // end extern "C" 

int  crank_mainpoint_entry(int  argc, std::string  argv[]);
int  crank_mainpoint_entry(int  argc, std::string  argv[])
{
SDL_Init(32);bool  quit = true;
;void * window = SDL_CreateWindow("This is a window", 805240832, 805240832, 1024, 768, 0);
;while (quit==true) 
{
Event  event;
;while (SDL_PollEvent(&event)) 
{
if (event.type==256) 
{
printf("Bye!\n");quit=true;}


}

}

SDL_Quit();return 0;
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
