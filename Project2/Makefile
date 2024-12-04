# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
HOME_DIR = Home
PROCESSED_DIR = Processed
UNPROCESSED_DIR = UnProcessed
BACKUP_DIR = Backup

# Source and object files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Targets for executables
MAIN_TARGET = $(BIN_DIR)/file_management_simulation
GENERATOR_TARGET = $(BIN_DIR)/file_generator
CALCULATOR_TARGET = $(BIN_DIR)/file_calculator
MOVER_TARGET = $(BIN_DIR)/file_mover
TYPE1_TARGET = $(BIN_DIR)/file_type1_inspector
TYPE2_TARGET = $(BIN_DIR)/file_type2_inspector
TYPE3_TARGET = $(BIN_DIR)/file_type3_inspector

# Default target
all: $(MAIN_TARGET) $(GENERATOR_TARGET) $(CALCULATOR_TARGET) $(MOVER_TARGET)  $(TYPE1_TARGET)  $(TYPE2_TARGET)  $(TYPE3_TARGET)

# Build the main referee executable
$(MAIN_TARGET): $(OBJ_DIR)/main.o  $(OBJ_DIR)/config.o  $(OBJ_DIR)/gui.o | $(BIN_DIR)
	$(CC) $^ -o $(MAIN_TARGET) -lGL -lGLU -lglut -lm # Linking OpenGL libraries


# Build the file generator executable 
$(GENERATOR_TARGET): $(OBJ_DIR)/file_generator.o $(OBJ_DIR)/config.o  | $(BIN_DIR)
	$(CC) $^ -o $(GENERATOR_TARGET)


# Build the file calculator executable 
$(CALCULATOR_TARGET): $(OBJ_DIR)/file_calculator.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(CALCULATOR_TARGET)

# Build the file mover executable 
$(MOVER_TARGET): $(OBJ_DIR)/file_mover.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(MOVER_TARGET)

# Build the file type1_inspector executable 
$(TYPE1_TARGET): $(OBJ_DIR)/file_type1_inspector.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(TYPE1_TARGET)

# Build the file type2_inspector executable 
$(TYPE2_TARGET): $(OBJ_DIR)/file_type2_inspector.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(TYPE2_TARGET)

# Build the file type3_inspector executable 
$(TYPE3_TARGET): $(OBJ_DIR)/file_type3_inspector.o $(OBJ_DIR)/config.o | $(BIN_DIR)
	$(CC) $^ -o $(TYPE3_TARGET)


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
