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
MAIN_TARGET = $(BIN_DIR)/bungee_simulation
ASSISTANT_TARGET = $(BIN_DIR)/assistant_referee
PLAYER_TARGET = $(BIN_DIR)/player

# Default target
all: $(MAIN_TARGET) $(ASSISTANT_TARGET) $(PLAYER_TARGET)

# Build the main referee executable
$(MAIN_TARGET): $(OBJ_DIR)/main_referee.o  $(OBJ_DIR)/config.o  $(OBJ_DIR)/gui.o | $(BIN_DIR)
	$(CC) $^ -o $(MAIN_TARGET) -lGL -lGLU -lglut -lm # Linking OpenGL libraries

# Build the assistant referee executable (no main function)
$(ASSISTANT_TARGET): $(OBJ_DIR)/assistant_referee.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(ASSISTANT_TARGET)

# Build the player executable (no main function)
$(PLAYER_TARGET): $(OBJ_DIR)/player.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(PLAYER_TARGET)

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


