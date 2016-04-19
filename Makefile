CC          = g++
CFLAGS      = -Wall -ansi -pedantic -ggdb -std=c++0x -g -O3
OBJS        = board.o chain.o mctree.o search.o
ENGINENAME  = go-engine

all: gtp

gtp: $(OBJS) gtp.o
	$(CC) -o $(ENGINENAME)$(EXT) $^

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@

clean:
	rm -f *.o $(ENGINENAME)$(EXT).exe $(ENGINENAME)$(EXT)
