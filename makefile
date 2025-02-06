CC = gcc
FLAGS = -Wall -Wextra -pedantic -g -lm

.PHONY: bin demo clean

bin:
	@mkdir -p bin

demo: src/demo.c src/hashmap/hashmap.c src/hashmap/hashmap.h | bin 
	$(CC) -o bin/demo $^ $(FLAGS)

clean:
	rm -rf bin

