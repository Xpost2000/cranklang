CC=g++

all: crank.exe
crank.exe: crank_parsing_tests.cc crank.cc tokenizer.h tokenizer.cc cplusplus_codegen.cc
	$(CC) tokenizer.cc crank.cc -ggdb3 -Wall -Wno-write-strings -o $@
