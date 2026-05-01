CMP = gcc
CFLAGS = -O2 -Wall
SRC := $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

build: $(OBJ)
	$(CMP) $^ -o bin/meow

build/%.o: src/%.c
	$(CMP) $(CFLAGS) -c $< -I ./include -o $@

clean: 
	rm -f build/*

.PHONY: build clean