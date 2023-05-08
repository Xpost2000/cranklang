# CrankLang

A simple alternative C language that transpiles.

**With no formal compiler education, I jump into writing a compiler, which turns out reasonably well all things considered.**
**NOTE the language is simple because this is my first actual programming language implemented effectively blind!**
**If I ever make a future language, I'll probably read the Dragon Book or take a look at LCC or something! Since there's probably a lot of things I shouldn't have done**

The codebase is kind of ugly as it evolves slightly, but it's reasonably okay to refactor compared to Legends anyways.


## Feature List Summary
    
- Handmade statically typed language that compiles to C++.
- Handmade recursive descent parser
- Constant folding
- **Multiple file through C style #include**, although there is automatic guarding against multiple include.
- Expression parsing and correct operator precedence order.
- **A simple FFI which allows you to use C functions**
- Out of order function declaration
- Record types such as unions and structs
- Scoped enums.
- For loops, if statements, while loops
- Binary and hexadecimal literals for easily writing flags.
- Dynamic arrays
- Object literals (and designated struct initializers)
- **Automatic pointer dereferencing!**

## Wishlist/Later
- Multiple modules (right now I'm just doing it like include)
- Array programming
- Bootstrapping/Self-hosting

## Technial Description

This is my own personal compiler educational project for the intent of
properly exercising my parsing muscles, because all I've written were
lisp interpreters. It's effectively a BetterC, and aims to just extend
a little on top of C.  The primary target is to have an extremely
simple to parse language that has incredibly consistent syntax. The
only implementation currently transpiles into C++, however the
compiler does do all the heavy lifting of typechecking and other
obligatory static analysis.

Crank is sort of a drop-in C replacement, and should be easy to
migrate to once you get used to do the changes to the syntax style
which are meant to be as ambiguous as possible.

**This is a toy language, so please don't try to actually use this.**

## Development / Compiling

Like all of my projects, I use an almost Unity build setup. There's a makefile that you use
to compile the project this time. It's not anything special, all you do is run make.

There are no dependencies other than a working C++ 11 compiler.

## Example programs

In the repository there are some test programs, although there is one serious example program.

In ```ExamplePong```, there is a two player game of Pong implemented in Crank with the help
of [raylib](https://github.com/raysan5/raylib). The game tries to use as many features of the language
as possible, and uses multiple files.

Since Crank doesn't include operating system awareness right now, building file expects Windows for now.
Although since Crank internally compiles to C++ it should be very easy to figure out how to change the build file
to work on other operating systems.

## Usage

```
crank --dotests
```
Runs some basic litmus tests on the type system and lexical analyzer.

```
crank --help
```
Will print out a help message. If I had one.

```
crank --output
```
Sets the name of the output file

```
crank --keepcpp
```
This is mainly a debug option, but it allows you to keep the compiled C++ code.

```
crank --link
```
Passes a library to link to the C++ compiler

```
crank --libdir
```
Passes a linking directory to the C++ compiler

### Example of proper usage
```
crank --output game --link SDL2 --libdir where_sdl2_is/ game.crank
```
