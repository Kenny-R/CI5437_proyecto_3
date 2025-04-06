#ifndef SUDOKUSOLVER_H
#define SUDOKUSOLVER_H

#include <vector>
#include <string>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <iostream>



int const D = 3;
int const N = D * D;

std::string parse_sudoku_to_DIMACS(std::string sudoku_path);

std::string parse_model_to_solution(std::map<int, bool> model);

#endif // !SUDOKUSOLVER_H