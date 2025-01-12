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
MAIN_TARGET = $(BIN_DIR)/main_program
COUNTER_ESPIONAGE_AGENCY_TARGET = $(BIN_DIR)/counter_espionage_agency
RESISTANCE_GROUP_TARGET = $(BIN_DIR)/resistance_group
ENEMY_TARGET = $(BIN_DIR)/enemy
CITIZEN_TARGET = $(BIN_DIR)/citizen

# Default target
all: $(MAIN_TARGET) $(COUNTER_ESPIONAGE_AGENCY_TARGET) $(RESISTANCE_GROUP_TARGET) $(ENEMY_TARGET) $(CITIZEN_TARGET)

# Build the main referee executable
$(MAIN_TARGET): $(OBJ_DIR)/main.o  $(OBJ_DIR)/config.o  | $(BIN_DIR)
	$(CC) $^ -o $(MAIN_TARGET) -lGL -lGLU -lglut -lm # Linking OpenGL libraries

$(COUNTER_ESPIONAGE_AGENCY_TARGET): $(OBJ_DIR)/counter_espionage_agency.o $(OBJ_DIR)/config.o  | $(BIN_DIR)
	$(CC) $^ -o $(COUNTER_ESPIONAGE_AGENCY_TARGET)

$(RESISTANCE_GROUP_TARGET): $(OBJ_DIR)/resistance_group.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(RESISTANCE_GROUP_TARGET)


$(ENEMY_TARGET): $(OBJ_DIR)/enemy.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(ENEMY_TARGET)

$(CITIZEN_TARGET): $(OBJ_DIR)/citizen.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(CITIZEN_TARGET)


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
	rm -f /tmp/fifo_*
	rm -f /tmp/project3_fifo_public
