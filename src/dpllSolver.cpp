#include "dpllSolver.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <set>

std::vector<std::vector<int>> parse_DIMACS_to_clauses(std::string dimacs)
{
	std::vector<std::vector<int>> clauses;
	std::vector<int> clause;
	std::istringstream iss(dimacs);
	std::string line;

	while (std::getline(iss, line))
	{
		// Ignorar líneas que comienzan con 'c', 'p' o '%'
		if (line.empty() || line[0] == 'c' || line[0] == 'p' || line[0] == '%')
		{
			continue;
		}

		std::istringstream iss2(line);
		int lit;
		while (iss2 >> lit)
		{
			if (lit == 0)
			{
				// Fin de una cláusul
				// esto es que tenga la forma a b c ... 0
				// si empieza con 0 ignoramos
				if (!clause.empty())
				{
					clauses.push_back(clause);
					clause.clear();
				}
			}
			else
			{
				// Añadir literal a la cláusula actual
				clause.push_back(lit);
			}
		}
	}

	// si la ultima clausula no tiene un 0 igual lo tomamos como valido
	if (!clause.empty())
	{
		clauses.push_back(clause);
	}

	return clauses;
}

std::pair<std::set<int>, std::set<int>> get_unit_clauses_and_pure_literals(const std::vector<std::vector<int>> &clauses)
{
	std::set<int> unit_clauses;
	std::map<int, int> literal_counts;
	std::set<int> pure_literals;

	for (const auto &clause : clauses)
	{
		if (clause.empty())
		{
			throw std::runtime_error("Se encontró una cláusula vacía, la fórmula es insatisfacible.");
		}

		if (clause.size() == 1)
		{
			unit_clauses.insert(clause[0]);
		}

		for (int literal : clause)
		{
			literal_counts[literal]++;
			if (literal_counts[-literal] > 0)
			{
				pure_literals.erase(literal);
				pure_literals.erase(-literal);
			}
			else
			{
				pure_literals.insert(literal);
			}
		}
	}

	return {unit_clauses, pure_literals};
}

std::vector<std::vector<int>> simplify(const std::vector<std::vector<int>> &clauses, int literal, bool value)
{
	// Si la vaina esta tarda mucho se puede hacer que esta funcion calcule los literales puros y las clausulas unitarias
	std::vector<std::vector<int>> simplified_clauses;

	for (const auto &clause : clauses)
	{
		bool clause_satisfied = false;
		std::vector<int> new_clause;

		for (int lit : clause)
		{
			if (lit == literal && value)
			{
				clause_satisfied = true;
				break;
			}
			else if (lit == -literal && !value)
			{
				clause_satisfied = true;
				break;
			}
			else if (lit != literal && lit != -literal)
			{
				new_clause.push_back(lit);
			}
		}

		if (!clause_satisfied)
		{
			if (!new_clause.empty())
			{
				simplified_clauses.push_back(new_clause);
			}
			else
			{
				// Si la cláusula se ha quedado vacía, la fórmula es insatisfacible
				throw std::runtime_error("Se encontró una cláusula vacía, la fórmula es insatisfacible.");
			}
		}
	}

	return simplified_clauses;
}

bool dpll_solver_rec(std::vector<std::vector<int>> &clauses, std::map<int, bool> &model)
{

	if (clauses.empty())
	{
		// La fórmula es satisfacible
		return true;
	}

	try
	{
		auto [unit_clauses, pure_literals] = get_unit_clauses_and_pure_literals(clauses);

		if (!pure_literals.empty())
		{
			int pure_literal = *pure_literals.begin();
			model[abs(pure_literal)] = (pure_literal > 0);
			clauses = simplify(clauses, abs(pure_literal), pure_literal > 0);
			return dpll_solver_rec(clauses, model);
		}

		if (!unit_clauses.empty())
		{
			int unit_clause = *unit_clauses.begin();
			model[abs(unit_clause)] = (unit_clause > 0);
			clauses = simplify(clauses, abs(unit_clause), unit_clause > 0);
			return dpll_solver_rec(clauses, model);
		}

		// Escoger un literal arbitrario
		int literal = clauses[0][0];
		model[abs(literal)] = (literal > 0);
		std::vector<std::vector<int>> clauses_copy = clauses;
		clauses = simplify(clauses, abs(literal), literal > 0);
		if (dpll_solver_rec(clauses, model))
		{
			return true;
		}
		else
		{
			model[abs(literal)] = !(literal > 0);
			clauses = simplify(clauses_copy, abs(literal), !(literal > 0));
			return dpll_solver_rec(clauses, model);
		}
	}
	catch (const std::runtime_error &e)
	{
#ifdef DEBUG
		std::cerr << "Error: " << e.what() << std::endl;
#endif // DEBUG
		return false;
	}
}

std::pair<bool, std::map<int, bool>> dpll_solver(std::string dimacs_file_path)
{

	// Read the file content into a string
	std::ifstream file(dimacs_file_path);
	if (!file)
	{
		std::cerr << "Error: Could not open file " << dimacs_file_path << std::endl;
		return {false, {}};
	}

	std::string dimacs_clauses((std::istreambuf_iterator<char>(file)),
							   std::istreambuf_iterator<char>());
	file.close();

	// Convert the DIMACS string into clauses
	std::vector<std::vector<int>> clauses = parse_DIMACS_to_clauses(dimacs_clauses);

	// Solve the problem using the DPLL solver
	std::map<int, bool> model;

	bool result = dpll_solver_rec(clauses, model);
	
	return {result, model};
}
