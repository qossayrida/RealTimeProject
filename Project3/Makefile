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


# Default target
all: $(MAIN_TARGET)

# Build the main referee executable
$(MAIN_TARGET): $(OBJ_DIR)/main.o  $(OBJ_DIR)/config.o  | $(BIN_DIR)
	$(CC) $^ -o $(MAIN_TARGET) -lGL -lGLU -lglut -lm # Linking OpenGL libraries


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
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(HOME_DIR) $(PROCESSED_DIR) $(UNPROCESSED_DIR) $(BACKUP_DIR) 
