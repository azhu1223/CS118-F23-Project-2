CC=g++
CFLAGS=-Wall -Wextra -std=c++20

all: clean build

default: build

build: server.cpp client.cpp
	${CC} ${CFLAGS} -o server server.cpp
	${CC} ${CFLAGS} -o client client.cpp

clean:
	rm -f server client output.txt project2.zip

zip: 
	zip project2.zip server.cpp client.cpp utils.h report.txt Makefile README
