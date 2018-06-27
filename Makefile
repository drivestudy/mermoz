CC = g++
OPT = -O3 -pthread
INC = -I src/.

all: examples

examples: urlparser

urlparser: examples/urlparser.cpp
	$(CC) $(OPT) $(INC) -o examples/$@ $^
