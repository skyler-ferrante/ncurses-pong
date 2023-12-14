CXX	:=gcc
CXXFLAGS:=-O2 -Wall
LDFLAGS	:=-lncurses -lm

TARGET	:=pong

.phony: run all clean

all: ${TARGET}

sounds.c: sounds/makesounds
	cd sounds ; ./makesounds ; mv sounds.c ..

clean: 
	rm -rf ${TARGET} sounds.o

.c.o:
	$(CC) -c  -I. $(CFLAGS) $(OPTIONS) $<

${TARGET}: % : %.c sounds.o
	${CXX} ${CXXFLAGS} $^ ${LDFLAGS}  -o $@
