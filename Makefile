# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wno-reorder -Wall -I./src/include

# Source files
DPLL_SRCS = src/dpllSolver.cpp src/dpllSolverMain.cpp 
SUDOKU_SRCS = src/sudokuSolver.cpp src/sudokuSolverMain.cpp src/dpllSolver.cpp

# Object files directory
BUILD_DIR = build

# Object files
DPLL_OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(DPLL_SRCS))
SUDOKU_OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SUDOKU_SRCS))

# Executable names
DPLL_EXEC = dpllSolver
SUDOKU_EXEC = sudokuSolver

# Default target
all: dpll sudoku

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build dpllSolver executable
dpll: $(DPLL_EXEC)

$(DPLL_EXEC): $(DPLL_OBJS) | $(BUILD_DIR)
	$(CXX) $(DPLL_OBJS) -o $@

# Build sudokuSolver executable
sudoku: $(SUDOKU_EXEC)

$(SUDOKU_EXEC): $(SUDOKU_OBJS) | $(BUILD_DIR)
	$(CXX) $(SUDOKU_OBJS) -o $@

# Compile source files into object files
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run dpllSolver
run-dpll: dpll
	./$(DPLL_EXEC)

# Run sudokuSolver
run-sudoku: sudoku
	./$(SUDOKU_EXEC)

# Clean up build files (without removing build directory)
clean:
	rm -f $(BUILD_DIR)/*.o $(DPLL_EXEC) $(SUDOKU_EXEC)

# Phony targets
.PHONY: all clean dpll sudoku run-dpll run-sudoku