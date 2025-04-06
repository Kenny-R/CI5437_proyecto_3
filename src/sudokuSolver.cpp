#include "sudokuSolver.h"

/*
 * El codigo utilizado para pasar de un sudoku a un archivo DIMACS fue tomado de: 
 * https://users.aalto.fi/~tjunttil/2020-DP-AUT/notes-sat/solving.html
*/

int var(int r, int c, int v)
{
	if (!(1 <= r && r <= N && 1 <= c && c <= N && 1 <= v && v <= N))
	{
		throw std::out_of_range("Entrada invalida:\nCondición '1 <= r <= N, 1 <= c <= N, 1 <= v <= N' not met. "
								"Values are: r = " +
								std::to_string(r) +
								", c = " + std::to_string(c) +
								", v = " + std::to_string(v) +
								", N = " + std::to_string(N));
	}
	return (r - 1) * N * N + (c - 1) * N + (v - 1) + 1;
}

std::string parse_sudoku_to_DIMACS(std::string sudoku_path)
{
	std::ifstream file(sudoku_path);
	std::vector<std::string> clues = {};
	std::vector<std::vector<int>> clauses = {};

	std::map<char, int> digits = {
		{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}};

	if (!file.is_open())
	{
		throw std::runtime_error("No se pudo abrir el archivo: " + sudoku_path);
	}

	std::string line;
	while (std::getline(file, line))
	{
		clues.push_back(line);
	}
	file.close();

	// Si el sudoku tiene una sola fila, lo convertimos a 9 filas
	if (clues.size() == 1 && clues[0].size() == 81)
    {
        std::string single_line = clues[0];
        clues.clear();
        for (int i = 0; i < 81; i += 9)
        {
            clues.push_back(single_line.substr(i, 9));
        }
    }

	if (clues.size() != N)
	{
		throw std::runtime_error("El número de filas en el archivo no coincide con el tamaño esperado N.");
	}

	std::vector<int> new_clause;
	for (int r = 1; r <= N; r++)
	{
		for (int c = 1; c <= N; c++)
		{
			// La celdas de la cuadricula deben tener al menos un valor
			new_clause = {};
			for (int v = 1; v <= N; v++)
			{
				new_clause.push_back({var(r, c, v)});
			}
			clauses.push_back(new_clause);

			// No puede haber dos valores en la misma celda
			for (int v = 1; v <= N; v++)
			{
				for (int w = v + 1; w <= N; w++)
				{
					clauses.push_back({-var(r, c, v), -var(r, c, w)});
				}
			}
		}
	}

	for (int v = 1; v <= N; v++)
	{

		// Cada fila debe tener el valor v
		for (int r = 1; r <= N; r++)
		{
			new_clause = {};
			for (int c = 1; c <= N; c++)
			{
				new_clause.push_back(var(r, c, v));
			}
			clauses.push_back(new_clause);
		}

		// Cada columna debe tener el valor v
		for (int c = 1; c <= N; c++)
		{
			new_clause = {};
			for (int r = 1; r <= N; r++)
			{
				new_clause.push_back(var(r, c, v));
			}
			clauses.push_back(new_clause);
		}

		// Cada subcuadricula debe tener el valor v
		for (int sr = 0; sr < D; sr++)
		{
			for (int sc = 0; sc < D; sc++)
			{
				new_clause = {};
				for (int rd = 1; rd <= D; rd++)
				{
					for (int cd = 1; cd <= D; cd++)
					{
						new_clause.push_back(var(sr * D + rd, sc * D + cd, v));
					}
				}
				clauses.push_back(new_clause);
			}
		}
	}

	// las pistas se respetan
	for (int r = 1; r <= N; r++)
	{
		for (int c = 1; c <= N; c++)
		{
			if (clues[r - 1][c - 1] != '.' && digits.find(clues[r - 1][c - 1]) != digits.end())
			{
				int value = digits[clues[r - 1][c - 1]];
				clauses.push_back({var(r, c, value)});

				// Posible mejora: agregar la negación de los demás valores
				// for (int v = 1; v <= N; v++)
				// {
				// 	if (v != value)
				// 	{
				// 		clauses.push_back({-var(r, c, v)});
				// 	}
				// }
			}
		}
	}

	std::string dimacs = "p cnf " + std::to_string(N * N * N) + " " + std::to_string(clauses.size()) + "\n";
	for (const auto &clause : clauses)
	{
		for (int literal : clause)
		{
			dimacs += std::to_string(literal) + " ";
		}
		dimacs += "0\n";
	}
	return dimacs;
}


std::string parse_model_to_solution(std::map<int, bool> model)
{
	std::string solution = "";
	for (int r = 1; r <= N; r++)
	{
		for (int c = 1; c <= N; c++)
		{
			int found_value = 0;
			for (int v = 1; v <= N; v++)
			{
				if (model[var(r, c, v)])
				{
					if (found_value != 0)
					{
						std::cerr << "Error: Se encontró más de un valor positivo en la celda (" 
								  << r << ", " << c << ")." << std::endl;
						return "";
					}
					found_value = v;
				}
			}
			if (found_value == 0)
			{
				std::cerr << "Error: No se encontró ningún valor positivo en la celda (" 
						  << r << ", " << c << ")." << std::endl;
				return "";
			}
			solution += std::to_string(found_value);
		}
	}
	return solution;
}