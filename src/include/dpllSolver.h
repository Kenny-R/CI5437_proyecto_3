#ifndef DPLL_SOLVER_H
#define DPLL_SOLVER_H
#include <vector>
#include <string>
// #define DEBUG

std::vector<std::vector<int>> parse_DIMACS_to_clauses(std::string dimacs);
// std::vector<std::vector<int>> &clauses, std::vector<int> &model
std::pair<bool, std::map<int, bool>> dpll_solver(std::string dimacs_file_path);

#endif // !DPLL_SOLVER_H
