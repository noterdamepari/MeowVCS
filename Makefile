CMP := gcc
CFLAGS := -O2 -Wall
INCLUDES := -I ./include -I./lib/zlib
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=build/%.o)
LIB := $(wildcard lib/*.a)

build: $(OBJ)
	$(CMP) $^ $(LIB) -o bin/meow

build/%.o: src/%.c
	$(CMP) $(CFLAGS) -c $< -I ./include -o $@

clean: 
	rm -f build/*

.PHONY: build clean