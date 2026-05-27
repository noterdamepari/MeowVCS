CMP := gcc
CFLAGS := -O2
INCLUDES := -I./include -I./lib/zlib

SRC := $(shell find src -name "*.c")
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
DEPS := $(OBJ:.o=.d)

LIB := $(wildcard lib/*.a)
TARGET := bin/meow

build: $(TARGET)

dbg: CFLAGS := -O2 -Wall -Wextra -DDEBUG -g3
dbg: build

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CMP) $(OBJ) $(LIB) -o $(TARGET)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CMP) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
	rm -rf build bin

-include $(DEPS)

.PHONY: build clean dbg
