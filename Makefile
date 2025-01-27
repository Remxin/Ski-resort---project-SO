# Paths and compiler setup
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I$(INCLUDE_DIR)

# Executables
MAIN_EXEC = main_program
SKIER_EXEC = skier
SKIER_GENERATOR_EXEC = skier_generator
CASHIER_EXEC = cashier
WORKER1_EXEC = worker1
WORKER2_EXEC = worker2

# Common source files
COMMON_SRC = $(SRC_DIR)/utils.c $(SRC_DIR)/ticket.c $(SRC_DIR)/platform.c $(SRC_DIR)/shared_memory.c

# Object files
COMMON_OBJ = $(COMMON_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN_OBJ = $(BUILD_DIR)/main.o $(COMMON_OBJ)
SKIER_OBJ = $(BUILD_DIR)/skier.o $(COMMON_OBJ)
CASHIER_OBJ = $(BUILD_DIR)/cashier.o $(COMMON_OBJ)
SKIER_GENERATOR_OBJ = $(BUILD_DIR)/skier_generator.o $(COMMON_OBJ)
WORKER1_OBJ = $(BUILD_DIR)/worker1.o $(COMMON_OBJ)
WORKER2_OBJ = $(BUILD_DIR)/worker2.o $(COMMON_OBJ)

# Create directories
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BIN_DIR))

# Main targets
all: $(MAIN_EXEC) $(SKIER_EXEC) $(CASHIER_EXEC) $(SKIER_GENERATOR_EXEC) $(WORKER1_EXEC) $(WORKER2_EXEC)

$(MAIN_EXEC): $(MAIN_OBJ)
	$(CC) $(MAIN_OBJ) -o $(BIN_DIR)/$(MAIN_EXEC)

$(SKIER_EXEC): $(SKIER_OBJ)
	$(CC) $(SKIER_OBJ) -o $(BIN_DIR)/$(SKIER_EXEC)

$(CASHIER_EXEC): $(CASHIER_OBJ)
	$(CC) $(CASHIER_OBJ) -o $(BIN_DIR)/$(CASHIER_EXEC)

$(SKIER_GENERATOR_EXEC): $(SKIER_GENERATOR_OBJ)
	$(CC) $(SKIER_GENERATOR_OBJ) -o $(BIN_DIR)/$(SKIER_GENERATOR_EXEC)

$(WORKER1_EXEC): $(WORKER1_OBJ)
	$(CC) $(WORKER1_OBJ) -o $(BIN_DIR)/$(WORKER1_EXEC)

$(WORKER2_EXEC): $(WORKER2_OBJ)
	$(CC) $(WORKER2_OBJ) -o $(BIN_DIR)/$(WORKER2_EXEC)


# Object files compilation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(MAIN_EXEC)
	./$(BIN_DIR)/$(MAIN_EXEC)

# Clean
clean:
	rm -rf $(BUILD_DIR)/*.o $(BIN_DIR)/*

.PHONY: all clean run