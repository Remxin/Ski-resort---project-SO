# Paths
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I$(INCLUDE_DIR)
EXEC = skier_program

SRC = $(SRC_DIR)/main.c $(SRC_DIR)/skier.c $(SRC_DIR)/utils.c
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)




$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(BIN_DIR)/$(EXEC)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/skier.o: $(SRC_DIR)/skier.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/skier.c -o $(BUILD_DIR)/skier.o

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/utils.c -o $(BUILD_DIR)/utils.o


run:
	./$(BIN_DIR)/$(EXEC)
# Clean structure
clean:
	rm -rf $(BUILD_DIR)/* $(BIN_DIR)/*

.PHONY: all clean