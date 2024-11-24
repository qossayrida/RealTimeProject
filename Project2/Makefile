# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source and object files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Targets for executables
MAIN_TARGET = $(BIN_DIR)/file_management_simulation
GENERATOR_TARGET = $(BIN_DIR)/file_generator
CALCULATOR_TARGET = $(BIN_DIR)/file_calculator

# Default target
all: $(MAIN_TARGET) $(GENERATOR_TARGET) $(CALCULATOR_TARGET)

# Build the main referee executable
$(MAIN_TARGET): $(OBJ_DIR)/main.o  $(OBJ_DIR)/config.o  $(OBJ_DIR)/gui.o | $(BIN_DIR)
	$(CC) $^ -o $(MAIN_TARGET) -lGL -lGLU -lglut -lm # Linking OpenGL libraries


# Build the file generator executable 
$(GENERATOR_TARGET): $(OBJ_DIR)/file_generator.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(GENERATOR_TARGET)


# Build the file calculator executable 
$(CALCULATOR_TARGET): $(OBJ_DIR)/file_calculator.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(CALCULATOR_TARGET)



# Compile each source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run the main_referee executable with the config file
run: $(MAIN_TARGET)
	$(MAIN_TARGET) config.txt

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)


