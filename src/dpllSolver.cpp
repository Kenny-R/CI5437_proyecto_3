#include "dpllSolver.h"

#define VERSION 3

/*
########################################################################################
#  Primera implementacion del DPLL Solver.
# Basado totalmente en el pseudocodigo visto en clases.
# Pros:
#  - Es intuitivo y facil de entender.
#  - Es facil de implementar.
#  - Es facil de modificar.
# Contras:
#  - Es recursivo.
#  - Es lento. (Debido a que hace varias copias cada vez que se llama a la funcion)
# Pruebas:
#  - Puede resolver problemas de unas 50 variables y 218 clausulas.
#  - No puede resolver problemas de 250 variables y 1065 clausulas.
########################################################################################
*/
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

/**
 * @brief Identifica las cláusulas unitarias y los literales puros en un conjunto de cláusulas.
 *
 * Esta función analiza un conjunto de cláusulas para encontrar:
 * - Cláusulas unitarias: aquellas que contienen un único literal.
 * - Literales puros: aquellos que aparecen únicamente con un signo (positivo o negativo) en todas las cláusulas.
 *
 * @param clauses Un vector de cláusulas, donde cada cláusula es un vector de literales (enteros).
 *                Los literales positivos representan variables verdaderas, y los negativos representan variables falsas.
 *
 * @return std::pair<std::set<int>, std::set<int>>
 *         - El primer elemento del `pair` es un conjunto de literales que representan las cláusulas unitarias.
 *         - El segundo elemento del `pair` es un conjunto de literales que representan los literales puros.
 *
 * @throws std::runtime_error Si:
 *         - Se encuentra una cláusula vacía, lo que indica que la fórmula es insatisfacible.
 *
 * @note Ejemplo de entrada:
 *       - Cláusulas: {{1, -3}, {2}, {-1, 3}, {4}}
 *
 * @note Ejemplo de salida:
 *       - Cláusulas unitarias: {2, 4}
 *       - Literales puros: {2, 3, 4}
 */
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

/**
 * @brief Simplifica un conjunto de cláusulas eliminando aquellas satisfechas y reduciendo las que contienen literales opuestos.
 *
 * Esta función toma un conjunto de cláusulas y un literal con su valor asignado. Elimina las cláusulas que son satisfechas
 * por el literal y reduce las cláusulas que contienen el literal opuesto, eliminándolo de dichas cláusulas.
 *
 * @param clauses Un vector de cláusulas, donde cada cláusula es un vector de literales (enteros).
 *                Los literales positivos representan variables verdaderas, y los negativos representan variables falsas.
 * @param literal Un entero que representa el literal que se está evaluando.
 * @param value Un booleano que indica el valor asignado al literal (`true` para verdadero, `false` para falso).
 *
 * @return std::vector<std::vector<int>>
 *         - Un nuevo conjunto de cláusulas simplificado.
 *
 * @throws std::runtime_error Si:
 *         - Una cláusula se queda vacía después de la simplificación, lo que indica que la fórmula es insatisfacible.
 *
 * @note Ejemplo de entrada:
 *       - Cláusulas: {{1, -3}, {2, -1}, {-2, 3}}
 *       - Literal: 1
 *       - Valor: true
 *
 * @note Ejemplo de salida:
 *       - Cláusulas simplificadas: {{-2, 3}}
 */
std::vector<std::vector<int>> simplify(const std::vector<std::vector<int>> &clauses, int literal, bool value)
{
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

/**
 * @brief Implementación recursiva del algoritmo DPLL para resolver problemas de satisfacibilidad booleana (SAT).
 *
 * Esta función utiliza el algoritmo DPLL para determinar si un conjunto de cláusulas
 * en formato CNF (Conjunción de Disyunciones) es satisfacible. La función opera de manera recursiva, asignando valores
 * a las variables y simplificando las cláusulas hasta encontrar una solución o determinar que no es posible satisfacerlas.
 *
 * @param clauses Un vector de cláusulas, donde cada cláusula es un vector de literales (enteros).
 *                Los literales positivos representan variables verdaderas, y los negativos representan variables falsas.
 * @param model Un mapa que representa el modelo actual (asignación de valores a las variables).
 *              Las claves son las variables, y los valores son booleanos.
 *
 * @return bool
 *         - `true` si el conjunto de cláusulas es satisfacible.
 *         - `false` si el conjunto de cláusulas no es satisfacible.
 *
 * @throws std::runtime_error Si:
 *         - Se encuentra una cláusula vacía durante la simplificación, lo que indica que la fórmula es insatisfacible.
 *
 * @note Proceso general:
 *       1. Si no hay cláusulas, la fórmula es satisfacible.
 *       2. Identificar literales puros y cláusulas unitarias, asignarles valores y simplificar las cláusulas.
 *       3. Si no hay literales puros ni cláusulas unitarias, seleccionar un literal arbitrario y probar ambas asignaciones.
 *       4. Repetir recursivamente hasta encontrar una solución o determinar que no es posible satisfacer la fórmula.
 *
 * @note Ejemplo de entrada:
 *       - Cláusulas: {{1, -3}, {2}, {-1, 3}, {4}}
 *       - Modelo inicial: {}
 *
 * @note Ejemplo de salida:
 *       - Resultado: `true`
 *       - Modelo: {1=true, 2=true, 3=false, 4=true}
 */
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

/*
#########################################################################################
#  Segunda implementacion del DPLL Solver.
# Basado en el pseudocodigo visto en clases.
# Pros:
#  - Sigue siendo intuitivo pero ya no es tan facil de entender
#  - Es mas eficiente que la primera implementacion. (hace solo una copia de la lista de clausulas cada vez que se llama a la funcion)
# Contras:
#  - Es un poco mas complicado de implementar.
#  - Es un poco mas complicado de modificar.
#  - Sigue siendo recursivo.
#  - Sigue siendo lento (Supongo que la culpa esta en la copia)
# Pruebas:
#  - Puede resolver problemas de unas 50 variables y 218 clausulas.
#  - No puede resolver problemas de 250 variables y 1065 clausulas.
##########################################################################################
*/

/**
 * @brief Segunda implementación recursiva del algoritmo DPLL para resolver problemas de satisfacibilidad booleana (SAT).
 *
 * Esta función utiliza una versión optimizada del algoritmo DPLL para determinar si un conjunto de cláusulas
 * en formato CNF (Conjunción de Disyunciones) es satisfacible. La función opera de manera recursiva, asignando valores
 * a las variables y simplificando las cláusulas hasta encontrar una solución o determinar que no es posible satisfacerlas.
 *
 * @param clauses Un vector de cláusulas, donde cada cláusula es un vector de literales (enteros).
 *                Los literales positivos representan variables verdaderas, y los negativos representan variables falsas.
 * @param symbols Un conjunto de enteros que representa las variables disponibles para asignar valores.
 * @param model Un mapa que representa el modelo actual (asignación de valores a las variables).
 *              Las claves son las variables, y los valores son booleanos (`true` para verdadero, `false` para falso).
 *
 * @return bool
 *         - `true` si el conjunto de cláusulas es satisfacible.
 *         - `false` si el conjunto de cláusulas no es satisfacible.
 *
 * @throws std::runtime_error Si:
 *         - Se encuentra una cláusula vacía durante la evaluación, lo que indica que la fórmula es insatisfacible.
 *
 * @note Proceso general:
 *       1. Si todas las cláusulas son verdaderas, la fórmula es satisfacible.
 *       2. Identificar literales puros y asignarles valores.
 *       3. Identificar cláusulas unitarias y asignarles valores.
 *       4. Si no hay literales puros ni cláusulas unitarias, seleccionar un literal arbitrario y probar ambas asignaciones.
 *       5. Repetir recursivamente hasta encontrar una solución o determinar que no es posible satisfacer la fórmula.
 *
 * @note Ejemplo de entrada:
 *       - Cláusulas: {{1, -3}, {2}, {-1, 3}, {4}}
 *       - Símbolos: {1, 2, 3, 4}
 *       - Modelo inicial: {}
 *
 * @note Ejemplo de salida:
 *       - Resultado: `true`
 *       - Modelo: {1=true, 2=true, 3=false, 4=true}
 */
bool dpll_solver_rec_mk2(std::vector<std::vector<int>> &clauses, std::set<int> symbols, std::map<int, bool> model)
{
	std::set<int> unit_clauses;
	std::set<int> pure_literals;
	std::map<int, int> literal_counts;
	std::set<int> unvalued_literals;
	int true_clauses = 0;
	bool empty_clause = true;

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
			return false;
		}

		if (unvalued_literals.size() == 1)
		{
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

		return dpll_solver_rec_mk2(clauses, symbols, model);
	}

	// si llegaste aquí no hay clausulas unitarias ni literales puros
	// por lo que vamos a asignar un literal arbitrario
	// y volvemos a ejecutar la funcion

	if (symbols.empty())
	{
		// si llegaste aquí no hay más símbolos para asignar
		// por lo que la fórmula es insatisfacible con la asignación que tienes
		return false;
	}
	int literal = symbols.extract(symbols.begin()).value();

	// model sera para el caso en que el literal sea verdadero

	std::map<int, bool> model_copy = model;
	model[abs(literal)] = (literal > 0);

	bool result = dpll_solver_rec_mk2(clauses, symbols, model);

	if (result)
	{
		return result;
	}
	else
	{
		model[abs(literal)] = !(literal > 0);
		return dpll_solver_rec_mk2(clauses, symbols, model);
	}
}

/*
##########################################################################################
#  Tercera implementacion del DPLL Solver.
#
# Mejora al pseudocodigo visto en clases.
# Pros:
#  - Es intuitivo y facil de entender.
#  - Es facil de implementar.
#  - Es iterativo
#  - Es eficiente (no hace copias, pero si pasa por todas las variables, esto puede llegar
#	 a ser un pequeño problema)
#  Contras:
#  - No utiliza los literales puros
# Pruebas:
#  - Puede resolver problemas de unas 50 variables y 218 clausulas.
#  - Puede resolver problemas de 250 variables y 1065 clausulas.
#  - No se pudo verficar si puede resolver la instancia de un sudoku de 9x9.
##########################################################################################
*/

// Valores globales que no van a cambiar

#define UNASSIGNED -1

#define TRUE 1

#define FALSE 0

/**
 * indica que el siguiente elemento en la pila del backstack es un literal del cual ya se tomo una decision.
 */
#define DECISION_MARK 0

/**
 * Una cantidad que se le suma a la actividad de un literal cada vez que este es parte de un conflicto.
 * Esta actividad se usa para decidir que literal se va a tomar como decision.
 * En pocas palabras se utiliza para tener una heuristica de decision. Tomaremos el literal que haya tenido
 * mas conflictos, para solucionarlo lo mas pronto posible.
 */
#define ACTIVITY_INCREMENT 1.0

/**
 * Para que la heuristica funcion hay que dividir entre 2 el valor de la actividad cada vez que
 * se llega a un conflicto. Esto es para darle chance a otros literales de ser elegidos como decision.
 * En este caso se hace cada 1000 conflictos.
 */
#define ACT_INC_UPDATE_RATE 1000

// Variables globales que se utilizan en varias funciones

uint num_variables;

uint num_clauses;

std::vector<std::vector<int>> clauses;

/**
 * Una lista que contiene las clausulas donde un literal aparece de manera positiva.
 */
std::vector<std::vector<std::vector<int> *>> positive_clauses;

/**
 * Una lista que contiene las clausulas donde un literal aparece de manera negativa.
 */
std::vector<std::vector<std::vector<int> *>> negative_clauses;

std::vector<int> model;

/**
 * una pila que contendra todas las decisiones que se han tomado.
 */
std::vector<int> model_stack;

/**
 * El indice de la siguiente literal que se va a propagar.
 */
uint index_of_next_literal_to_propagate;

/**
 * la profundidad del arbol de decision.
 */
uint decision_level;

/**
 * Cantidad de conflictos que se han encontrado donde un literal esta en forma positiva.
 */
std::vector<double> positive_literal_activity;

/**
 * Cantidad de conflictos que se han encontrado donde un literal esta en forma negativa.
 */
std::vector<double> negative_literal_activity;

uint conflicts;

uint propagations;

uint decisions;

inline uint literal_var(int literal)
{
	return abs(literal);
}

/**
 * @brief Parsea un archivo en formato DIMACS y lo convierte en una representación interna para el algoritmo DPLL.
 *
 * Esta función lee un archivo en formato DIMACS, que contiene un problema de satisfacibilidad booleana (SAT),
 * y lo convierte en una estructura interna que incluye las cláusulas, las variables y otras estructuras necesarias
 * para ejecutar el algoritmo DPLL.
 *
 * @param file_path Una cadena de texto que contiene la ruta del archivo en formato DIMACS.
 *                  Este archivo debe incluir:
 *                  - Comentarios opcionales que comienzan con 'c'.
 *                  - Una línea de encabezado que comienza con 'p' y especifica el número de variables y cláusulas.
 *                  - Las cláusulas, donde cada línea contiene literales separados por espacios y termina con un '0'.
 *
 * @throws std::runtime_error Si:
 *         - El archivo no se puede abrir.
 *         - El archivo no cumple con el formato DIMACS esperado.
 *
 * @note Ejemplo de entrada en formato DIMACS:
 *       c Este es un comentario
 *       p cnf 3 2
 *       1 -3 0
 *       2 3 -1 0
 *
 * @note Estructuras internas generadas:
 *       - `clauses`: Un vector de cláusulas, donde cada cláusula es un vector de literales (enteros).
 *       - `positive_clauses`: Una lista de las cláusulas donde cada literal aparece de forma positiva.
 *       - `negative_clauses`: Una lista de las cláusulas donde cada literal aparece de forma negativa.
 *       - `model`: Un vector que representa el modelo actual (asignación de valores a las variables).
 *       - `model_stack`: Una pila que contiene las decisiones tomadas durante la ejecución del algoritmo.
 *       - `positive_literal_activity` y `negative_literal_activity`: Actividad de los literales para la heurística de decisión.
 */
void parse_DIMACS_input(const std::string &file_path)
{
	std::ifstream file(file_path);
	if (!file.is_open())
	{
		throw std::runtime_error("Error: No se puede abrir el archivo: " + file_path);
	}

	// Si hay comentarios los ignoramos
	char c = file.get();
	while (c == 'c')
	{
		while (c != '\n')
			c = file.get();
		c = file.get();
	}

	std::string aux;
	file >> aux >> num_variables >> num_clauses;
	clauses.resize(num_clauses);

	positive_clauses.resize(num_variables + 1);
	negative_clauses.resize(num_variables + 1);

	for (uint clause = 0; clause < num_clauses; ++clause)
	{
		int literal;
		while (file >> literal && literal != 0)
		{
			clauses[clause].push_back(literal);

			// llenamos las listas de apariciones positivas y negativas
			if (literal > 0)
			{
				positive_clauses[literal_var(literal)].push_back((std::vector<int> *)&clauses[clause]);
			}
			else
			{
				negative_clauses[literal_var(literal)].push_back((std::vector<int> *)&clauses[clause]);
			}
		}
	}

	model.resize(num_variables + 1, UNASSIGNED);
	index_of_next_literal_to_propagate = 0;
	decision_level = 0;

	positive_literal_activity.resize(num_variables + 1, 0.0);
	negative_literal_activity.resize(num_variables + 1, 0.0);
	conflicts = 0;
	propagations = 0;
	decisions = 0;
}

/**
 * @brief Obtiene el valor de un literal en el modelo actual.
 *
 * Esta función evalúa el valor de un literal en el modelo actual. Si el literal es positivo, devuelve su valor directamente.
 * Si el literal es negativo, devuelve el valor opuesto al de su variable correspondiente en el modelo.
 *
 * @param literal Un entero que representa el literal a evaluar. Los literales positivos representan variables verdaderas,
 *                y los literales negativos representan variables falsas.
 *
 * @return int
 *         - `TRUE` (1) si el literal es verdadero.
 *         - `FALSE` (0) si el literal es falso.
 *         - `UNASSIGNED` (-1) si el literal no ha sido asignado en el modelo.
 */
int get_literal_value(int literal)
{
	if (literal >= 0)
	{
		return model[literal];
	}
	else
	{
		if (model[-literal] == UNASSIGNED)
		{
			return UNASSIGNED;
		}
		else
		{
			return 1 - model[-literal];
		}
	}
}

/**
 * @brief Asigna un literal como verdadero en el modelo actual.
 *
 * Esta función actualiza el modelo actual para reflejar que un literal ha sido asignado como verdadero.
 * Si el literal es positivo, se asigna como verdadero directamente. Si el literal es negativo, se asigna
 * como falso su variable correspondiente. Además, el literal se agrega a la pila de decisiones (`model_stack`).
 *
 * @param literal Un entero que representa el literal a asignar como verdadero.
 *                Los literales positivos representan variables verdaderas,
 *                y los literales negativos representan variables falsas.
 */
void set_literal_to_true(int literal)
{
	// Si entramos aquí estamos tomando una decisión
	// por eso es que agregamos el literal a la pila
	model_stack.push_back(literal);

	if (literal > 0)
	{
		model[literal] = TRUE;
	}
	else
	{
		model[-literal] = FALSE;
	}
}

/**
 * @brief Actualiza la actividad asociada a un literal.
 *
 * Esta función incrementa la actividad de un literal en función de su signo
 * (positivo o negativo). Se utiliza un índice calculado a partir del literal
 * para determinar qué vector de actividad actualizar.
 *
 * @param literal Un entero que representa el literal cuya actividad
 *                debe ser actualizada. Si es positivo, incrementará la actividad
 *                positiva; si es negativo, incrementará la actividad negativa.
 *
 * @note Se asume que las variables globales `positive_literal_activity` y
 *       `negative_literal_activity` están previamente definidas y son accesibles
 *       dentro del ámbito de esta función.
 *
 */
void update_activity_literal(int literal)
{
	uint index = literal_var(literal);
	if (literal > 0)
	{
		positive_literal_activity[index] += ACTIVITY_INCREMENT;
	}
	else
	{
		negative_literal_activity[index] += ACTIVITY_INCREMENT;
	}
}

/**
 * @brief Actualiza la actividad de los literales en una cláusula conflictiva.
 *
 * Esta función se invoca cuando se detecta una cláusula conflictiva durante el
 * proceso de resolución. Incrementa el contador de conflictos y realiza un ajuste
 * en las actividades de todos los literales si se cumple una tasa de actualización
 * definida. Finalmente, actualiza las actividades de los literales de la cláusula dada.
 *
 * @param clause Un vector de enteros que representa la cláusula conflictiva.
 *               Cada entero corresponde a un literal en la cláusula.
 *
 * @details
 *  - Si el número de conflictos alcanza un múltiplo de `ACT_INC_UPDATE_RATE`,
 *    las actividades de todos los literales se reducen a la mitad.
 *  - Para cada literal en la cláusula, se invoca la función
 *    `update_activity_literal` para incrementar su actividad específica.
 *
 * @note Las variables globales `conflicts`, `positive_literal_activity`,
 *       `negative_literal_activity`, `ACT_INC_UPDATE_RATE` y `num_variables`
 *       deben estar correctamente inicializadas y configuradas antes de llamar
 *       a esta función.
 */
void update_activity_conflicting_clause(const std::vector<int> &clause)
{
	++conflicts;
	if (conflicts % ACT_INC_UPDATE_RATE == 0)
	{
		for (uint i = 1; i <= num_variables; ++i)
		{
			positive_literal_activity[i] /= 2.0;
			negative_literal_activity[i] /= 2.0;
		}
	}

	for (int literal : clause)
	{
		update_activity_literal(literal);
	}
}
/**
 * @brief Propaga los literales en el modelo para identificar posibles conflictos.
 *
 * Esta función realiza la propagación de literales en la pila del modelo para
 * verificar si existen conflictos en las cláusulas asociadas. Si se encuentra
 * una cláusula conflictiva, se actualizan las actividades heurísticas y se
 * devuelve un valor de conflicto. En caso contrario, asigna valores a los
 * literales no asignados en cláusulas unitarias.
 *
 * @return `true` si se detecta un conflicto; `false` si no se encuentra ninguno.
 *
 * @details
 * - La propagación recorre la pila del modelo (`model_stack`) y evalúa los literales.
 * - Para cada literal, filtra las cláusulas conflictivas en función del estado lógico
 *   del literal y verifica si alguna cláusula se ha quedado vacía.
 * - Las cláusulas vacías indican un conflicto y se manejan con la función
 *   `update_activity_conflicting_clause`.
 * - Las cláusulas unitarias llevan a la asignación de su único literal no asignado.
 *
 * @note Las variables globales `index_of_next_literal_to_propagate`, `model_stack`,
 *       `negative_clauses`, `positive_clauses`, `propagations`, y las funciones como
 *       `get_literal_value`, `update_activity_conflicting_clause`, y `set_literal_to_true`
 *       deben estar correctamente definidas y configuradas antes de invocar esta función.
 */
bool propagate_conflicts()
{
	while (index_of_next_literal_to_propagate < model_stack.size())
	{
		// Tomamos el utltimo literal que se agrego a la pila, este sera el literal
		// que se va a propagar si hay un conflicto
		int literal_to_propagate = model_stack[index_of_next_literal_to_propagate];

		++index_of_next_literal_to_propagate;

		// Esto no es necesario si no se muestra al usuario lo puedo quitar sin problemas
		++propagations;

		// Aquí vamos a filtrar las clausulas que vamos a revisar a solo en las que el
		// el literal se vuelve false, esto por que un "conflicto" es cuando una clausula
		// esta vacia (osea todos los valores son false)
		std::vector<std::vector<int> *> clauses_to_propagate = literal_to_propagate > 0 ? negative_clauses[literal_var(literal_to_propagate)] : positive_clauses[literal_var(literal_to_propagate)];

		for (const auto &clause_ptr : clauses_to_propagate)
		{
			const std::vector<int> &clause = *clause_ptr;

			bool is_some_literal_true = false;
			int unassigned_literal = 0;
			int last_unassigned_literal = 0;

			for (int literal : clause)
			{
				int value = get_literal_value(literal);
				if (value == TRUE)
				{
					is_some_literal_true = true;
					break;
				}
				else if (value == UNASSIGNED)
				{
					++unassigned_literal;
					last_unassigned_literal = literal;
				}
			}

			if (not is_some_literal_true and unassigned_literal == 0)
			{
				// Si llegamos aquí es por que la clausula se ha quedado vacia
				// por lo tanto tenemos un conflicto

				// actualizamos los valores para la heuristica
				update_activity_conflicting_clause(clause);
				return true;
			}
			else if (not is_some_literal_true and unassigned_literal == 1)
			{
				// Si llegamos aquí es por que la clausula es unitaria
				// por lo tanto tenemos que asignar el valor del literal
				// y volver a verificar si no se creo un conflicto
				set_literal_to_true(last_unassigned_literal);
			}
		}
	}
	// Si llega aquí es por que no se ha encontrado un conflicto
	return false;
}

/**
 * @brief Retrocede en el nivel de decisión y ajusta las decisiones tomadas.
 *
 * Esta función se utiliza cuando el camino de decisiones tomado no lleva a una
 * solución. Revertirá las decisiones realizadas en el nivel actual, desasignará
 * los literales correspondientes y ajustará los valores para poder explorar
 * alternativas en el espacio de búsqueda.
 *
 * @details
 * - La función recorre la pila del modelo (`model_stack`) desde el final hasta
 *   encontrar un marcador de decisión (`DECISION_MARK`), deshaciendo las
 *   asignaciones de los literales.
 * - Una vez alcanzado el marcador, ajusta el nivel de decisión (`decision_level`)
 *   y establece un literal inverso como verdadero para explorar otro camino.
 * - Actualiza la variable `index_of_next_literal_to_propagate` para reflejar el
 *   nuevo estado de la pila.
 *
 * @note Es importante que las variables globales `model_stack`, `model`,
 *       `decision_level`, y la función `set_literal_to_true` estén correctamente
 *       definidas antes de llamar a esta función.
 */
void backtrack()
{
	// Si el camino que escogimos no funciono, hay que echar para atras
	// eso es lo que hace esta función, olvida las decisiones que tomamos por el camino
	// y cuando llega al inicio del camino que decidimos tomar, invierte el valor
	// para que podamos seguir investigando.

	uint i = model_stack.size() - 1;
	int literal = 0;
	while (model_stack[i] != DECISION_MARK)
	{
		literal = model_stack[i];
		model[literal_var(literal)] = UNASSIGNED;
		model_stack.pop_back();
		--i;
	}
	model_stack.pop_back();
	--decision_level;
	index_of_next_literal_to_propagate = model_stack.size();
	set_literal_to_true(-literal);
}


 /**
 * @brief Selecciona el siguiente literal para tomar como decisión heurística.
 *
 * Esta función implementa una heurística para escoger el siguiente literal a 
 * decidir. Por defecto, selecciona el literal con mayor actividad.
 *
 * @return El literal con mayor actividad que aún no ha sido asignado.
 *
 * @details
 * - Recorre todas las variables en el modelo para determinar cuál literal tiene 
 *   la mayor actividad, evaluando tanto su actividad positiva como negativa.
 * - Incrementa el contador de decisiones (`decisions`) para reflejar el número 
 *   de decisiones tomadas en el proceso de resolución.
 *
 * @note Es necesario que las variables globales `positive_literal_activity`, 
 *       `negative_literal_activity`, `model`, `num_variables` y la constante 
 *       `UNASSIGNED` estén correctamente configuradas antes de usar esta función.
 *
 * @todo Esta función creo que puede ser un poco costosa por que pasa por todas las variables
 * por lo que si no funciona para los sudokus lo que podemos hacer es revisar al azar, tipo tomar
 * 100 o 200 literales y revisar, y tomar el que tenga la mayor actividad.
 *
 */
int get_next_decision_literal()
{
	// Aquí se escoge el siguiente literal a tomar como decision
	// en este caso se escoge el que tenga la mayor actividad
	// Esta es la heuristica que se utiliza para decidir que literal.

	// esto tambien es algo que no importa mucho, si no se lo voy a mostrar al usuario lo puedo eliminar
	++decisions;

	double maximum_activity = 0.0;
	int most_active_literal = 0;
	for (uint i = 1; i <= num_variables; ++i)
	{
		if (model[i] == UNASSIGNED)
		{
			if (positive_literal_activity[i] >= maximum_activity)
			{
				maximum_activity = positive_literal_activity[i];
				most_active_literal = i;
			}
			else if (negative_literal_activity[i] >= maximum_activity)
			{
				maximum_activity = negative_literal_activity[i];
				most_active_literal = -i;
			}
		}
	}

	return most_active_literal;
}

/**
 * @brief Verifica si todas las cláusulas en el modelo son satisfechas.
 *
 * Esta función recorre todas las cláusulas del modelo para asegurarse de que 
 * al menos un literal en cada cláusula sea verdadero. Si encuentra una cláusula 
 * no satisfecha, imprime un mensaje de error con los literales de la cláusula 
 * conflictiva y finaliza el programa.
 *
 * @details
 * - Cada cláusula es evaluada para verificar si alguno de sus literales tiene 
 *   un valor de verdadero (`TRUE`).
 * - Si una cláusula no tiene ningún literal verdadero, se considera como no satisfecha.
 * - El programa muestra los literales de la cláusula que causa el error y se 
 *   termina inmediatamente usando `exit(1)`.
 *
 * @note Es necesario que las variables globales `clauses`, `num_clauses` y la función 
 *       `get_literal_value` estén correctamente inicializadas antes de llamar a esta función.
 */
void check_model()
{
	for (uint i = 0; i < num_clauses; ++i)
	{
		bool some_true = false;
		for (uint j = 0; not some_true and j < clauses[i].size(); ++j)
		{
			some_true = (get_literal_value(clauses[i][j]) == TRUE);
		}
		if (not some_true)
		{
			std::cout << "Error en el modelo, la clausula no es satisfecha:";
			for (uint j = 0; j < clauses[i].size(); ++j)
			{
				std::cout << clauses[i][j] << " ";
			}
			std::cout << std::endl;
			exit(1);
		}
	}
}
/**
 * @brief Finaliza el programa indicando la satisfacibilidad del modelo.
 *
 * Esta función imprime el estado del modelo en función de su satisfacibilidad y 
 * proporciona estadísticas relevantes como el número de decisiones y propagaciones 
 * realizadas durante el proceso. Además, si el modelo es satisfacible, verifica 
 * su validez utilizando la función `check_model`.
 *
 * @param satisfiable Un valor booleano que indica si el modelo es satisfacible 
 *                    (`true`) o no (`false`).
 *
 * @return `true` si el modelo es satisfacible, `false` en caso contrario.
 *
 * @details
 * - Cuando el modelo es satisfacible, la función llama a `check_model` para 
 *   confirmar su validez y luego imprime los resultados junto con las estadísticas.
 * - Si el modelo no es satisfacible, imprime el estado correspondiente junto con 
 *   las estadísticas y devuelve `false`.
 * - La salida estándar muestra los valores de `decisions` y `propagations` para 
 *   facilitar el análisis del proceso de resolución.
 *
 */

bool exit_with_satisfiability(bool satisfiable)
{
	if (satisfiable)
	{
		check_model();
		// std::cout << "SATISFACIBLE,"s << decisions << "," << propagations << std::endl;
		return true;
	}
	else
	{
		// std::cout << "NO SE PUEDE SATISFACER," << decisions << "," << propagations << std::endl;
		return false;
	}
}

/**
 * @brief Ejecuta el algoritmo DPLL para determinar la satisfacibilidad del problema.
 *
 * Esta función implementa el núcleo del algoritmo DPLL 
 * para resolver problemas de satisfacibilidad lógica. Alterna entre la propagación de 
 * conflictos y la toma de decisiones, retrocediendo cuando es necesario, hasta que se 
 * determina si el problema es satisfacible o insatisfacible.
 *
 * @return `true` si el problema es satisfacible, `false` en caso contrario.
 *
 * @details
 * - Durante cada iteración, intenta propagar los literales para detectar conflictos.
 * - Si la propagación identifica un conflicto y no hay niveles de decisión restantes, 
 *   se considera el problema como insatisfacible.
 * - Si no se detectan conflictos y no quedan literales para decidir, el problema se 
 *   considera satisfacible.
 * - En caso de que haya literales para decidir, se toma una nueva decisión y se 
 *   continúa el proceso.
 *
 * @note Requiere que las funciones `propagate_conflicts`, `backtrack`, `get_next_decision_literal`, 
 *       `exit_with_satisfiability` y `set_literal_to_true`, así como las variables globales 
 *       `model_stack`, `DECISION_MARK`, `index_of_next_literal_to_propagate`, y `decision_level`, 
 *       estén correctamente configuradas antes de llamar a esta función.
 *
 */
bool execute_DPLL()
{
	while (true)
	{
		while (propagate_conflicts())
		{
			if (decision_level == 0)
			{
				// No hay más decisiones posibles, lo que significa que el problema es insatisfacible
				return exit_with_satisfiability(false);
			}
			backtrack();
		}

		int decision_literal = get_next_decision_literal();
		if (decision_literal == 0)
		{
			return exit_with_satisfiability(true);
		}

		// Aquí es donde ya tomamos una nueva decision
		model_stack.push_back(DECISION_MARK); // agregar marca indicando una nueva decision
		++index_of_next_literal_to_propagate;
		++decision_level;
		set_literal_to_true(decision_literal); // ahora agregar el primer literal
	}
}

/**
 * @brief Verifica y maneja cláusulas unitarias en el modelo.
 *
 * Esta función recorre todas las cláusulas del modelo en busca de aquellas que 
 * son unitarias (es decir, que contienen solo un literal). Si encuentra una cláusula 
 * unitaria que es falsa, determina que el modelo es insatisfacible y finaliza el programa. 
 * Si encuentra una cláusula unitaria con un literal no asignado, asigna dicho literal 
 * como verdadero.
 *
 * @details
 * - Las cláusulas unitarias son evaluadas para verificar su estado lógico:
 *   - Si el literal es falso, el modelo es insatisfacible.
 *   - Si el literal está sin asignar, se le asigna un valor verdadero.
 * - Esta verificación asegura que el modelo cumpla las restricciones de las cláusulas 
 *   unitarias antes de proceder con la resolución.
 *
 * @note Requiere que las variables globales `clauses`, `num_clauses` y las funciones 
 *       `get_literal_value`, `set_literal_to_true` y `exit_with_satisfiability` estén 
 *       correctamente definidas y configuradas antes de llamar a esta función.
 *
 */
void check_unit_clauses()
{
	for (uint i = 0; i < num_clauses; ++i)
	{
		if (clauses[i].size() == 1)
		{
			int literal = clauses[i][0];
			int value = get_literal_value(literal);
			if (value == FALSE)
			{
				// Si ya de una encontramos que hay una clausula unitaria falsa
				// no hay que buscar mas, esto significa que por algun lugar esta
				// el mismo literal pero con el signo cambiado lo que hace
				// que la expresion siempre sea falsa
				exit_with_satisfiability(false);
			}
			else if (value == UNASSIGNED)
			{
				set_literal_to_true(literal);
			}
		}
	}
}

bool main_test(const std::string &file_path)
{
	// Read the problem file (available at the stdin stream) and
	//  initialize the rest of necessary variables
	parse_DIMACS_input(file_path);

	// Take care of initial unit clauses, if any
	check_unit_clauses();

	// Execute the main DPLL procedure
	return execute_DPLL();
}


/**
 * @brief Resuelve un problema de satisfacibilidad lógica utilizando el algoritmo DPLL.
 *
 * Esta función sirve como punto de entrada principal para el solucionador DPLL, 
 * procesando un archivo DIMACS que representa el problema lógico y devolviendo 
 * el resultado de la satisfacibilidad junto con el modelo resultante.
 *
 * @param dimacs_file_path Ruta al archivo DIMACS que contiene la representación 
 *                         del problema de satisfacibilidad.
 *
 * @return Un par (`std::pair`) que contiene:
 *         - Un valor booleano que indica si el problema es satisfacible (`true`) 
 *           o insatisfacible (`false`).
 *         - Un mapa (`std::map<int, bool>`) que representa el modelo resultante 
 *           de asignaciones de literales si el problema es satisfacible.
 *
 * @details
 * - Si el archivo DIMACS no puede abrirse, se imprime un mensaje de error y se 
 *   devuelve un par vacío con un resultado `false`.
 *
 * @note Las funciones auxiliares como `parse_DIMACS_to_clauses`, `check_unit_clauses`, 
 *       y `execute_DPLL`, así como las variables globales necesarias, deben estar 
 *       correctamente definidas antes de usar esta función.
 * 
 */
std::pair<bool, std::map<int, bool>> dpll_solver(std::string dimacs_file_path)
{

	std::cout << "Solving " << dimacs_file_path << std::endl;

	if (VERSION == 1)
	{
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

		bool result = dpll_solver_rec(clauses, model);

		return {result, model};
	}
	else if (VERSION == 2)
	{
		// Read the DIMACS file and parse it into clauses
		std::ifstream file(dimacs_file_path);
		if (!file)
		{
			std::cerr << "Error: Could not open file " << dimacs_file_path << std::endl;
			return {false, {}};
		}

		std::string dimacs_clauses((std::istreambuf_iterator<char>(file)),
								   std::istreambuf_iterator<char>());
		file.close();

		auto [clauses, symbols] = parse_DIMACS_to_clauses(dimacs_clauses);

		std::map<int, bool> model;

		bool result = dpll_solver_rec_mk2(clauses, symbols, model);

		return {result, model};
	}
	else if (VERSION == 3)
	{
		parse_DIMACS_input(dimacs_file_path);

		// Take care of initial unit clauses, if any
		check_unit_clauses();

		// Execute the main DPLL procedure
		bool result = execute_DPLL();

		std::map<int, bool> model_map;
		for (uint i = 1; i <= num_variables; ++i)
		{
			if (model[i] != UNASSIGNED)
			{
				model_map[i] = (model[i] == TRUE);
			}
		}
		return {result, model_map};
	}
	else
	{
		std::cerr << "Error: Version no valida" << std::endl;
		return {false, {}};
	}
}
