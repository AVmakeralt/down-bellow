# The Voided - prototype build
#
# Targets:
#   make            build the game binary at build/voided
#   make run        build + run with cwd at project root
#   make clean      remove build artifacts
#   make preview    (re)render the art preview PNG via Python

CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2 -g
LDFLAGS ?=

# SDL2: prefer pkg-config if available, else fall back to manual paths
SDL2_CFLAGS ?= $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL2_LIBS   ?= $(shell pkg-config --libs   sdl2 2>/dev/null)
ifeq ($(SDL2_LIBS),)
    # Manual fallback (works on this dev box)
    SDL2_CFLAGS := -I/usr/include/SDL2
    SDL2_LIBS   := -lSDL2
endif

SRC_DIR := src
ART_DIR := art
BUILD   := build
BIN     := $(BUILD)/voided

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.o,$(SRCS))

INC := -I$(SRC_DIR) -I$(ART_DIR) $(SDL2_CFLAGS)
LIBS := $(SDL2_LIBS) -lm

.PHONY: all run clean preview

all: $(BIN)

$(BIN): $(OBJS) | $(BUILD)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

run: $(BIN)
	cd . && ./$(BIN)

preview:
	@command -v python3 >/dev/null || { echo "python3 required"; exit 1; }
	python3 /home/z/my-project/scripts/preview_art.py

clean:
	rm -rf $(BUILD)

# Header deps: rebuild all objects if any header changes
DEPS := $(wildcard $(SRC_DIR)/*.h) $(wildcard $(ART_DIR)/*.h)
$(OBJS): $(DEPS)
