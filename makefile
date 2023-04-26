CC=g++

all: crank.exe
crank.exe: codegen.cc cplusplus_codegen.cc crank_parsing_tests.cc debug_print.cc crank.cc tokenizer.h tokenizer.cc cplusplus_codegen.cc
	$(CC) tokenizer.cc crank.cc -ggdb3 -Wno-write-strings -o $@
