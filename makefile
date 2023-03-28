CC=g++

all: crank.exe
crank.exe: crank.cc tokenizer.h tokenizer.cc
	$(CC) tokenizer.cc crank.cc -ggdb3 -Wno-write-strings -o $@
