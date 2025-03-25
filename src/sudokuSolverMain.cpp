#include "sudokuSolver.h"

#include <iostream>
#include <string>
#include <stdexcept>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_sudoku>" << std::endl;
        return 1;
    }

    std::string sudoku_path = argv[1];

    try {
        // Procesar el archivo de Sudoku y generar el formato DIMACS
        std::string dimacs = parse_sudoku_to_DIMACS(sudoku_path);

        // Imprimir el formato DIMACS en la salida estándar
        std::cout << dimacs;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}