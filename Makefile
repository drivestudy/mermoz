CC = g++
OPT = -std=c++14 -Wall -O3 -pthread
#OPT = -std=c++14 -Wall -g -pthread
INC = -I src/. -I src/urlfactory/src/.
LIB = -lgumbo -lboost_program_options -lcurl
#PROF = -DMMZ_PROFILE
VERB = -DMMZ_VERBOSE

LIBMERMOZ = build/libmermoz.a
LIBURLFACTORY = src/urlfactory/build/liburlfactory.a

all: build

build: dir lib mermoz

dir:
	mkdir -p build

lib: $(LIBMERMOZ)

BINLIST = src/common/packer.o src/common/logs.o src/common/httpfetch.o\
					src/common/memsec.o src/urlserver/urlserver.o\
					src/spider/spider.o src/spider/parser.o\
					src/spider/fetcher.o

%.o: %.cpp
	$(CC) $(OPT) $(PROF) $(VERB) $(INC) -c -o $@ $^

$(LIBMERMOZ): $(BINLIST)
	ar rcs $@ $^

mermoz: src/mermoz.cpp
	$(CC) $(OPT) $(PROF) $(VERB) $(INC) -o build/$@ $^\
		$(LIBMERMOZ) $(LIBURLFACTORY) $(LIB) 

clean:
	rm -rf build src/common/*.o src/spider/*.o src/urlserver/*.o
