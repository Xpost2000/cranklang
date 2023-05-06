CC=g++

all: crank.exe
crank-dev.exe: src/codegen.cc src/cplusplus_codegen.cc src/crank_parsing_tests.cc src/debug_print.cc src/crank.cc src/tokenizer.h src/tokenizer.cc src/cplusplus_codegen.cc
	$(CC) src/tokenizer.cc src/crank.cc -ggdb3 -Wno-write-strings -o $@
crank.exe: src/codegen.cc src/cplusplus_codegen.cc src/crank_parsing_tests.cc src/debug_print.cc src/crank.cc src/tokenizer.h src/tokenizer.cc src/cplusplus_codegen.cc
	$(CC) src/tokenizer.cc src/crank.cc -DRELEASE -O2 -Wno-write-strings -o $@
