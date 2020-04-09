CXX	:=gcc
CXXFLAGS:=-O2
LDFLAGS	:=-lncurses

TARGET	:=pong

.phony: run all clean


all: ${TARGET}

clean: 
	rm -rf ${TARGET}

${TARGET}: % : %.c
	${CXX} -o $@ ${CXXFLAGS} ${LDFLAGS} $^
