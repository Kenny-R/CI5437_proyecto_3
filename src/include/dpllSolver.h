#ifndef DPLL_SOLVER_H
#define DPLL_SOLVER_H
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <csignal>
// #define DEBUG

/**
 * @brief Convierte el contenido de un archivo en formato DIMACS a una representación interna de cláusulas y variables.
 *
 * Esta función toma como entrada una cadena que contiene el contenido completo de un archivo en formato DIMACS.
 * El formato DIMACS es comúnmente utilizado para representar problemas de satisfacibilidad booleana (SAT).
 * La función procesa el contenido y devuelve un conjunto de cláusulas y un conjunto de variables.
 *
 * @param dimacs Una cadena de texto que contiene el contenido del archivo en formato DIMACS.
 *               Este texto debe incluir:
 *               - Comentarios opcionales que comienzan con 'c'.
 *               - Una línea de encabezado que comienza con 'p' y especifica el número de variables y cláusulas.
 *               - Las cláusulas, donde cada línea contiene literales separados por espacios y termina con un '0'.
 *
 * @return std::tuple<std::vector<std::vector<int>>, std::set<int>>
 *         - El primer elemento del `tuple` es un vector de cláusulas, donde cada cláusula es un vector de literales (enteros).
 *         - El segundo elemento es un conjunto de variables presentes en las cláusulas.
 *
 * @throws std::runtime_error Si:
 *         - La cantidad de cláusulas no coincide con la especificada en el encabezado.
 *         - Se encuentran variables fuera del rango especificado en el encabezado.
 *         - Se encuentra una cláusula vacía.
 *
 * @note Ejemplo de entrada en formato DIMACS:
 *       c Este es un comentario
 *       p cnf 3 2
 *       1 -3 0
 *       2 3 -1 0
 *
 * @note Ejemplo de salida:
 *       - Cláusulas: {{1, -3}, {2, 3, -1}}
 *       - Variables: {1, 2, 3}
 */
std::tuple<std::vector<std::vector<int>>, std::set<int>>  parse_DIMACS_to_clauses(std::string dimacs);

/**
 * @brief Resuelve un problema de satisfacibilidad booleana (SAT) dado en formato DIMACS.
 *
 * Esta función toma como entrada la ruta de un archivo en formato DIMACS, lo procesa y utiliza el algoritmo DPLL
 * para determinar si la fórmula booleana es satisfacible. Si es satisfacible, también devuelve el modelo que satisface la fórmula.
 *
 * @param dimacs_file_path Una cadena de texto que contiene la ruta del archivo en formato DIMACS.
 *                         Este archivo debe incluir:
 *                         - Comentarios opcionales que comienzan con 'c'.
 *                         - Una línea de encabezado que comienza con 'p' y especifica el número de variables y cláusulas.
 *                         - Las cláusulas, donde cada línea contiene literales separados por espacios y termina con un '0'.
 *
 * @return std::pair<bool, std::map<int, bool>>
 *         - El primer elemento del `pair` es un booleano que indica si la fórmula es satisfacible (`true`) o insatisfacible (`false`).
 *         - El segundo elemento es un mapa que representa el modelo (asignación de valores a las variables) si la fórmula es satisfacible.
 *           Si la fórmula es insatisfacible, este mapa estará vacío.
 *
 * @throws std::runtime_error Si:
 *         - El archivo no se puede abrir o leer.
 *         - El archivo no cumple con el formato DIMACS esperado.
 *
 * @note Ejemplo de entrada en formato DIMACS:
 *       c Este es un comentario
 *       p cnf 3 2
 *       1 -3 0
 *       2 3 -1 0
 *
 * @note Ejemplo de salida:
 *       - Si la fórmula es satisfacible:
 *         {true, {{1, true}, {2, false}, {3, true}}}
 *       - Si la fórmula es insatisfacible:
 *         {false, {}}
 */
std::pair<bool, std::map<int, bool>> dpll_solver(std::string dimacs_file_path);

bool main_test(const std::string &file_path);

#endif // !DPLL_SOLVER_H
