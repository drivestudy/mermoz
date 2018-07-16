CC = g++
OPT = -std=c++14 -O3 -pthread
#OPT = -std=c++14 -g -pthread
INC = -I src/.
LIB = -lgumbo -lboost_program_options -lcurl

LIBMERMOZ = build/libmermoz.a

all: build examples

build: dir lib mermoz

dir:
	mkdir -p build

lib: $(LIBMERMOZ)

BINLIST = src/common/urlparser.o src/common/packer.o src/common/logs.o\
					src/common/httpfetch.o src/common/memsec.o src/urlserver/urlserver.o\
					src/urlserver/robots.o src/spider/spider.o src/spider/parser.o\
					src/spider/fetcher.o

%.o: %.cpp
	$(CC) $(OPT) $(INC) -c -o $@ $^

$(LIBMERMOZ): $(BINLIST)
	ar rcs $@ $^

mermoz: src/mermoz.cpp
	$(CC) $(OPT) $(INC) -o build/$@ $^ $(LIBMERMOZ) $(LIB) 

examples: urlparser robots

urlparser: examples/urlparser.cpp
	$(CC) $(OPT) $(INC) -o examples/$@ $^ $(LIBMERMOZ)

robots: examples/robots.cpp
	$(CC) $(OPT) $(INC) -o examples/$@ $^ $(LIBMERMOZ) $(LIB)

clean:
	rm -rf build src/common/*.o src/spider/*.o src/urlserver/*.o
