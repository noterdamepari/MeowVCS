CMP := gcc
CFLAGS := -O2 -Wall
INCLUDES := -I./include -I./lib/zlib

SRC := $(shell find src -name "*.c")

OBJ := $(patsubst src/%.c, build/%.o, $(SRC))

LIB := $(wildcard lib/*.a)
TARGET := bin/meow

build: $(TARGET)

dbg: CFLAGS := -O2 -Wall -DDEBUG
dbg: build

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CMP) $(OBJ) $(LIB) -o $(TARGET)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CMP) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf build bin

.PHONY: build clean
