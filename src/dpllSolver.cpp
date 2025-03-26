#include "dpllSolver.h"
volatile bool debug_pause = true;

std::tuple<std::vector<std::vector<int>>, std::set<int>> parse_DIMACS_to_clauses(std::string dimacs)
{
	std::vector<std::vector<int>> clauses;
	std::vector<int> clause;
	std::set<int> variables;
	std::istringstream iss(dimacs);
	std::string line;
	int num_variables = 0;
	int num_clauses = 0;

	while (std::getline(iss, line))
	{
		// Ignorar líneas que comienzan con 'c', 'p' o '%'
		if (line.empty() || line[0] == 'c' || line[0] == '%')
		{
			continue;
		}

		if (line[0] == 'p')
		{
			// Línea de encabezado: p cnf <num_variables> <num_clauses>
			std::istringstream iss_header(line);
			std::string temp;
			iss_header >> temp >> temp >> num_variables >> num_clauses;
			continue;
		}

		std::istringstream iss2(line);
		int lit;
		while (iss2 >> lit)
		{
			if (lit == 0)
			{
				// Fin de una cláusula
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
				variables.insert(std::abs(lit));
			}
		}
	}

	// Si la última cláusula no tiene un 0, igual la tomamos como válida
	if (!clause.empty())
	{
		clauses.push_back(clause);
	}

	// Verificar que la cantidad de cláusulas coincida con la especificada en el encabezado
	if (clauses.size() != num_clauses)
	{
		throw std::runtime_error("La cantidad de cláusulas no coincide con la especificada en el archivo.");
	}

	// Verificar que las variables estén dentro del rango especificado
	if (*variables.rbegin() > num_variables)
	{
		throw std::runtime_error("Se encontraron variables fuera del rango especificado en el archivo.");
	}

	return {clauses, variables};
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

bool dpll_solver_rec_mk2(std::vector<std::vector<int>> &clauses, std::set<int> symbols, std::map<int, bool> model)
{
	std::set<int> unit_clauses;
	std::set<int> pure_literals;
	std::map<int, int> literal_counts;
	std::set<int> unvalued_literals;
	int true_clauses = 0;
	bool empty_clause = true;
	// asm("int3");

	// Este for busca por todas las clausulas chequeando varias cosas.
	for (const auto &clause : clauses)
	{
		unvalued_literals.clear();
		for (int literal : clause)
		{
			if (model.find(abs(literal)) != model.end())
			{
				if ((literal > 0 && model[abs(literal)]) || (literal < 0 && !model[abs(literal)]))
				{	
					unvalued_literals.clear();
					true_clauses++;
					empty_clause = false;
					break;
				}
			}
			else
			{
				empty_clause = false;
				literal_counts[literal]++;
				unvalued_literals.insert(literal);
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


		if (empty_clause)
		{
			std::cout << "Clausula vacía encontrada." << std::endl;
			return false;
		}

		if (unvalued_literals.size() == 1)
		{
			// std::cout << "Literal no evaluado: ";
			// for (int lit : unvalued_literals)
			// {
			// 	std::cout << lit << " ";
			// }
			// std::cout << std::endl;

			// std::cout << "Modelo actual: ";
			// for (const auto &[key, value] : model)
			// {
			// 	std::cout << key << "=" << value << " ";
			// }
			// std::cout << std::endl;

			// std::cout << "Cláusula unitaria encontrada: ";
			// for (int lit : clause)
			// {
			// 	std::cout << lit << " ";
			// }
			// std::cout << std::endl;

			// std::cout << "Conteo de literales: ";
			// for (const auto &[literal, count] : literal_counts)
			// {
			// 	std::cout << literal << "=" << count << " ";
			// }
			// std::cout << std::endl;

			// asm("int3");

			unit_clauses.insert(*unvalued_literals.begin());
		}
	}

	if (true_clauses == clauses.size())
	{
		std::cout << "Todas las clausulas son verdaderas." << std::endl;
		return true;
	}

	// Si hay un literal puro lo asignamos y volvemos a ejecutar la funcion
	if (!pure_literals.empty())
	{
		int pure_literal = *pure_literals.begin();
		model[abs(pure_literal)] = (pure_literal > 0);
		symbols.erase(abs(pure_literal));
		std::cout << "Llamada recursiva con literales puros: " << pure_literal << std::endl;
		std::cout << "Símbolos restantes: ";
		for (int symbol : symbols)
		{
			std::cout << symbol << " ";
		}
		std::cout << std::endl;
		std::cout << "Modelo actual: ";
		for (const auto &[key, value] : model)
		{
			std::cout << key << "=" << value << " ";
		}
		std::cout << std::endl;

		std::cout << "Literales puros encontrados: ";
		for (int pure_literal : pure_literals)
		{
			std::cout << pure_literal << " ";
		}
		std::cout << std::endl;
		return dpll_solver_rec_mk2(clauses, symbols, model);
	}

	// si llegaste aquí ya no hay literales puros ahora vamos a
	// buscar si hay clausulas unitarias si hay una la asignamos
	// y volvemos a ejecutar la funcion
	if (!unit_clauses.empty())
	{
		int unit_clause = *unit_clauses.begin();
		model[abs(unit_clause)] = (unit_clause > 0);
		symbols.erase(abs(unit_clause));
		std::cout << "Llamada recursiva con clausula unitaria: " << unit_clause << std::endl;
		std::cout << "Símbolos restantes: ";
		for (int symbol : symbols)
		{
			std::cout << symbol << " ";
		}
		std::cout << std::endl;
		std::cout << "Modelo actual: ";
		for (const auto &[key, value] : model)
		{
			std::cout << key << "=" << value << " ";
		}
		std::cout << std::endl;
		return dpll_solver_rec_mk2(clauses, symbols, model);
	}

	// si llegaste aquí no hay clausulas unitarias ni literales puros
	// por lo que vamos a asignar un literal arbitrario
	// y volvemos a ejecutar la funcion

	if (symbols.empty())
	{
		// si llegaste aquí no hay más símbolos para asignar
		// por lo que la fórmula es insatisfacible con la asignación que tienes
		std::cout << "No hay más símbolos para asignar." << std::endl;
		// asm("int3");
		return false;
	}
	int literal = symbols.extract(symbols.begin()).value();

	// model sera para el caso en que el literal sea verdadero

	std::map<int, bool> model_copy = model;
	model[abs(literal)] = (literal > 0);

	std::cout << "Llamada con literal arbitrario: " << literal << std::endl;
	std::cout << "Símbolos restantes: ";
	for (int symbol : symbols)
	{
		std::cout << symbol << " ";
	}
	std::cout << std::endl;
	std::cout << "Modelo actual: ";
	for (const auto &[key, value] : model)
	{
		std::cout << key << "=" << value << " ";
	}
	std::cout << std::endl;
	std::cout << "Probando literal: " << literal << " con valor: " << (literal > 0) << std::endl;

	bool result = dpll_solver_rec_mk2(clauses, symbols, model);

	if (result)
	{
		return result;
	}
	else
	{
		std::cout << "No hubo solución con el valor anterior. Literal: " << literal << ", valor anterior: " << (literal > 0) << ". Probando con el valor: " << !(literal > 0) << std::endl;

		model[abs(literal)] = !(literal > 0);
		return dpll_solver_rec_mk2(clauses, symbols, model);
	}
}

std::pair<bool, std::map<int, bool>> dpll_solver(std::string dimacs_file_path)
{

	std::cout << "Solving " << dimacs_file_path << std::endl;
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
	auto [clauses, symbols] = parse_DIMACS_to_clauses(dimacs_clauses);

	// Solve the problem using the DPLL solver
	std::map<int, bool> model;

	// bool result = dpll_solver_rec(clauses, model);

	bool result = dpll_solver_rec_mk2(clauses, symbols, model);

	return {result, model};
}
