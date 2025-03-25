#include <iostream>
#include <fstream>
#include "include/dpllSolver.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <sudoku_file_path>" << std::endl;
		return 1;
	}

	std::string dimacs_file_path = argv[1];

	bool result = dpll_solver(dimacs_file_path);

	// Output the result
	if (result) {
		std::cout << "Satisfacible" << std::endl;
	} else {
		std::cout << "No solution exists for the given Sudoku." << std::endl;
	}

	return 0;
}