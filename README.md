# CrankLang

This is a basic alternate C-syntax sort of language.

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

The language is designed to transpile relatively easily to C, and
also includes an interpreter.

The base implementation of the transpiler is in C++. It is not implemented in the most efficient
way. Rather it was implemented in a way that I believe to be simple to read, and otherwise not too difficult
to modify.

## Interpreter Notes

As of now the interpreter is just based on evaluating the syntax tree that the
compiler produces.

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