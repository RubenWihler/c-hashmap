CC = gcc
FLAGS = -Wall -Wextra -pedantic -g

.PHONY: bin test clean

bin:
	@mkdir -p bin

test: src/main.c src/hashmap/hashmap.c src/hashmap/hashmap.h | bin 
	$(CC) -o bin/test $^ $(FLAGS)

clean:
	rm -rf bin

