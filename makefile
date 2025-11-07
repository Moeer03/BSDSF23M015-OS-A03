
# --------- Directory Variables ---------
SRC_DIR     := src
INCLUDE_DIR := include
OBJ_DIR     := obj
BIN_DIR     := bin

# --------- Compiler Settings -----------
CC      := gcc
CFLAGS  := -Wall -Wextra -I$(INCLUDE_DIR)
TARGET  := $(BIN_DIR)/myshell

# --------- Source and Object Files -----
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/shell.c $(SRC_DIR)/execute.c
OBJS := $(OBJ_DIR)/main.o $(OBJ_DIR)/shell.o $(OBJ_DIR)/execute.o

# =========================================================
# Default target — build the shell
# =========================================================
all: $(TARGET)
	@echo "✅ Build complete! Run the shell using: ./bin/myshell"

# =========================================================
# Linking stage
# =========================================================
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "🔗 Linking object files..."
	$(CC) $(OBJS) -o $(TARGET)
	@echo "🚀 Executable created at $(TARGET)"

# =========================================================
# Compilation stage
# =========================================================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "🛠️  Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

# =========================================================
# Directory creation (if not exist)
# =========================================================
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# =========================================================
# Clean targets
# =========================================================
clean:
	@echo "🧹 Cleaning object files and binaries..."
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "✨ Clean complete."

# =========================================================
# Run target
# =========================================================
run: all
	@echo "🚀 Running myshell..."
	./$(TARGET)

# =========================================================
# Phony targets
# =========================================================
.PHONY: all clean run
