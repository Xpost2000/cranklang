# CrankLang

**NOTE: Crank is now featureful enough to have a simple FFI spec, and other than typedefs from the standard library**
**the language only requires the existence of a C compiler. I still rely on std::vector for dynamic arrays but now the language is less "magic"**

**NOTE: most of this is actually just a "TODO list" right now, although at the moment a subset of
the language features compile.**

**NOTE: One fun target goal, is to get this to be "self-hosting". While I still depend on a C compiler since I'm
not going to pull in LLVM and writing my own native codegen is kind of painful, I'll consider being able to rewrite the compiler
in Crank to be a cool goal :)**

This is a basic alternate C-syntax sort of language. It's an exercise in writing parsers and figuring
out how to write compilers. So the language is effective a C+=0.5. It's effectively still C, but with some
extra features.

The language parses everything and should be typechecking so a C code generator isn't exactly a copout!

I don't really advise using this because it's a toy language but at least you
can keep the C++ code it compiles if something really goes south.

It's a project mostly for me to learn how to make a programming language, and
properly document a project because most of my other stuff is more off the cuff.

It supports most of the common features that make a language
usable, and additional features it implements on top of the
C-language include:

- Module System
- Anonymous Functions
- Arrays as proper types
- Array Programming (first class array objects) (all component wise operations.)

The language is designed to transpile relatively easily to C, and may include an interpreter. As I'm thinking carefully
of how to do a handrolled x86_64 ffi, however anything related to structs is causing me to draw blanks as I have to pack
records into C structs and unpack them from C structs as well... I may not include it for a while (or ever, it kind of depends on how
worth it I think is to do it.)

The base implementation of the transpiler is in C++. It is not implemented in the most efficient
way. Rather it was implemented in a way that I believe to be simple to read, and otherwise not too difficult
to modify.

## Transpiler Notes

The C++ dialect it compiles in will be C-ish style C++ because the language
features are oriented around that.

Also since C++ doesn't support some of the language features, some traces of the
original Crank code will disappear afterwards (for instance the module system only exists
during the compilation stage).

When compiling to C++ the language will produce code that requires the existance of a runtime spec,
which is implemented in C.

## Examples 

There are a few program examples included in this repository which are:

- Hello World
- Star Stairs
- The Game Of Life

# Build/Run/Install

There is an included makefile and as long as you have any sufficient C++ compiler this should
build fine.

GCC is used for the releases.
