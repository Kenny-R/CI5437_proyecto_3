#ifndef SUDOKUSOLVER_H
#define SUDOKUSOLVER_H

#include <vector>
#include <string>


int const D = 3;
int const N = D * D;

std::string parse_sudoku_to_DIMACS(std::string sudoku_path);


#endif // !SUDOKUSOLVER_H