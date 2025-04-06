#include <iostream>
#include <fstream>
#include <chrono>
#include "include/dpllSolver.h"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <sudoku_file_path>" << std::endl;
		return 1;
	}

	bool show_results = false;

	std::string dimacs_file_path = argv[1];

	// Start measuring time
	auto start_time = std::chrono::high_resolution_clock::now();

	auto [result, model] = dpll_solver(dimacs_file_path);

	// Stop measuring time
	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_time = end_time - start_time;

	// Output the result
	if (result)
	{
		std::cout << "SATISFIABLE" << std::endl;
	}
	else
	{
		std::cout << "UNSATISFIABLE" << std::endl;
	}

	// Output the elapsed time
	std::cout << "Time: " << elapsed_time.count() << " seconds" << std::endl;

	if (show_results && result)
	{
		char user_input;
		std::cout << "¿Desea ver los valores de las variables? (s/n): ";
		std::cin >> user_input;

		if (user_input == 's' || user_input == 'S')
		{
			std::cout << "Valores de las variables:" << std::endl;
			for (const auto &[variable, value] : model)
			{
				std::cout << "Variable " << variable << " = " << (value ? "true" : "false") << std::endl;
			}
		}
	}

	return 0;
}