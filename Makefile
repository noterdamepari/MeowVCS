all: build
	gcc build/main.o -o bin/meow

build: src/main.c
	gcc -c src/main.c -I ./include -o build/main.o

clean: 
	rm -f build/*

.PHONY: build