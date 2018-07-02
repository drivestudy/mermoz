CC = g++
OPT = -std=c++11 -O3 -pthread
OPT = -std=c++11 -g -pthread
INC = -I src/.
LIB = -lrdkafka++ -lgumbo -lboost_program_options -lcurl

LIBMERMOZ = build/libmermoz.a
LMMZ = -L $(LIBMERMOZ)

all: mermoz examples

mermoz: dir lib spider

dir:
	mkdir -p build

lib: $(LIBMERMOZ)

BINLIST = src/common/urlparser.o

%.o: %.cpp
	$(CC) $(OPT) $(INC) -c -o $@ $^

$(LIBMERMOZ): $(BINLIST)
	ar rcs $@ $^

spider: src/spider/spider.cpp src/spider/fetcher.cpp src/spider/parser.cpp
	$(CC) $(OPT) $(INC) -o build/$@ $^ $(LMMZ) $(LIB)

examples: urlparser

urlparser: examples/urlparser.cpp
	$(CC) $(OPT) $(INC) -o examples/$@ $^ $(LMMZ)
