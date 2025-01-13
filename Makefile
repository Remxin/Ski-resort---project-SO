# Paths
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I$(INCLUDE_DIR)

# Default target
all: $(TARGET)

# Link object files into the final binary
$(TARGET): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean structure
clean:
	rm -rf $(BUILD_DIR)/* $(BIN_DIR)/*

.PHONY: all clean install