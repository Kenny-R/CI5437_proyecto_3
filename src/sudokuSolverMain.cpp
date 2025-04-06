#include "sudokuSolver.h"
#include "dpllSolver.h" // Include the header file where dpll_solver is declared

#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_sudoku>" << std::endl;
        return 1;
    }

    std::string sudoku_path = argv[1];

    try {
        // Procesar el archivo de Sudoku y generar el formato DIMACS
        std::string dimacs = parse_sudoku_to_DIMACS(sudoku_path);

        // Generar un nombre de archivo único utilizando un identificador universal
        std::string unique_filename = "sudoku_dimacs_" + std::to_string(std::hash<std::string>{}(sudoku_path)) + "_" + std::to_string(::time(nullptr)) + ".cnf";

        // Guardar el contenido del formato DIMACS en el archivo
        std::ofstream dimacs_file(unique_filename);
        if (!dimacs_file) {
            throw std::runtime_error("No se pudo crear el archivo DIMACS: " + unique_filename);
        }
        dimacs_file << dimacs;
        dimacs_file.close();

        // Pasar la dirección del archivo al dpll_solver
        auto [result, model] = dpll_solver(unique_filename);


        // Mostrar el resultado
        if (result) {
            std::cout << "SATISFIABLE" << std::endl;
            std::string solution = parse_model_to_solution(model);
            std::cout << "Solución: " << solution << std::endl;
        } else {
            std::cout << "UNSATISFIABLE" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}