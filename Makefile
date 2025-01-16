# Paths
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I$(INCLUDE_DIR)
EXEC = skier_program

# Source files
SRC = $(SRC_DIR)/main.c \
      $(SRC_DIR)/skier.c \
      $(SRC_DIR)/utils.c \
      $(SRC_DIR)/cashier.c \
      $(SRC_DIR)/ticket.c \
      $(SRC_DIR)/shared_memory.c \
      $(SRC_DIR)/platform.c \
      $(SRC_DIR)/worker.c


# Object files
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Make sure build and bin directories exist
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BIN_DIR))

# Main target
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(BIN_DIR)/$(EXEC)

# Pattern rule for object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(INCLUDE_DIR)/skier.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/cashier.h \
                     $(INCLUDE_DIR)/ticket.h $(INCLUDE_DIR)/shared_memory.h $(INCLUDE_DIR)/config.h \
                     $(INCLUDE_DIR)/platform.h $(INCLUDE_DIR)/worker.h
$(BUILD_DIR)/skier.o: $(SRC_DIR)/skier.c $(INCLUDE_DIR)/skier.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/config.h
$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.c $(INCLUDE_DIR)/utils.h
$(BUILD_DIR)/cashier.o: $(SRC_DIR)/cashier.c $(INCLUDE_DIR)/cashier.h $(INCLUDE_DIR)/ticket.h $(INCLUDE_DIR)/config.h $(INCLUDE_DIR)/utils.h
$(BUILD_DIR)/ticket.o: $(SRC_DIR)/ticket.c $(INCLUDE_DIR)/ticket.h $(INCLUDE_DIR)/config.h $(INCLUDE_DIR)/utils.h
$(BUILD_DIR)/shared_memory.o: $(SRC_DIR)/shared_memory.c $(INCLUDE_DIR)/shared_memory.h $(INCLUDE_DIR)/config.h
$(BUILD_DIR)/platform.o: $(SRC_DIR)/platform.c $(INCLUDE_DIR)/platform.h $(INCLUDE_DIR)/config.h $(INCLUDE_DIR)/shared_memory.h
$(BUILD_DIR)/worker.o: $(SRC_DIR)/worker.c $(INCLUDE_DIR)/worker.h $(INCLUDE_DIR)/platform.h $(INCLUDE_DIR)/config.h

# Run the program
run: $(EXEC)
	./$(BIN_DIR)/$(EXEC)

# Clean structure
clean:
	rm -rf $(BUILD_DIR)/*.o $(BIN_DIR)/$(EXEC)

.PHONY: all clean run