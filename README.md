# Proyecto 3: Sudoku Solver

Este repositorio contiene el código fuente del Proyecto 2 de la materia Inteligencia Artificial 1 (CI5437) de la Universidad Simón Bolívar.

## Información del Curso

- **Materia:** Inteligencia Artificial 1 (CI5437)
- **Profesor:** Guillermo Palma

## Desarrolladores

- **Kenny Rojas**. 18-10595
- **Fredthery Castro**. 18-10783

## Compilación del Proyecto

Para compilar el proyecto, se utiliza el archivo [`Makefile`](Makefile). Este genera dos ejecutables:

1. **`dpllSolver`**: Permite probar el código del SAT Solver directamente.
2. **`sudokuSolver`**: Permite probar la resolución de sudokus utilizando el SAT Solver.

### Pasos para compilar:

1. Asegúrate de tener instalado un compilador compatible con C++17 (por ejemplo, `g++`).
2. Ejecuta el siguiente comando en la raíz del proyecto:

	```bash
	make
	```

Esto compilará el proyecto y generará los ejecutables en el directorio raíz.


### Limpieza de archivos genereados:
Para limpiar los archivos objeto y los ejecutables generados, ejecuta:
``` bash
make clean
```

Nota: La carpeta build no será eliminada durante la limpieza.

### Ejecución

- Para ejecutar el SAT Solver:

```Bash
./dpllSolver <archivo.cnf>
```

- Para ejecutar el Sudoku Solver:
```Bash
./sudokuSolver <archivo con el sudoku codificado>
```

### Script para resolver múltiples casos

El repositorio incluye un script de bash llamado [`run_dpplsolver.sh`](run_dpplsolver.sh) que permite resolver todos los casos en formato `.cnf` contenidos en una carpeta específica. 

#### Uso del script:

1. Asegúrate de que el script tenga permisos de ejecución:
	```bash
	chmod +x run_dpplsolver.sh
	```

2. Ejecuta el script:
	```bash
	./run_dpplsolver.sh
	```

3. Después de ejecutar el script, este te pedirá que ingreses la dirección de la carpeta que contiene los archivos `.cnf`.

El script procesará cada archivo `.cnf` en la carpeta utilizando el ejecutable `dpllSolver`.

## SAT Solver
El código del SAT Solver está implementado en los archivos [`dpllSolver.h`](src/include/dpllSolver.h) y [`dpllSolver.cpp`](src/dpllSolver.cpp). Este solver utiliza el algoritmo DPLL y cuenta con tres versiones distintas, cada una mejorando aspectos de eficiencia y claridad respecto a la anterior:

1. **Primera versión**: Es la más sencilla e intuitiva, basada directamente en el pseudocódigo visto en clases. Sin embargo, es menos eficiente debido a su naturaleza recursiva y a las copias innecesarias de datos.
2. **Segunda versión**: Introduce optimizaciones para reducir las copias de datos, lo que la hace más eficiente que la primera versión, aunque es un poco más compleja de entender y modificar.
3. **Tercera versión**: Es la más avanzada, implementada de forma iterativa para evitar problemas de recursión y mejorar la eficiencia. Además, utiliza heurísticas para la toma de decisiones, lo que la hace significativamente más rápida.

## Resolución de Sudokus

El código para resolver sudokus está implementado en los archivos [`sudokuSolver.h`](src/include/sudokuSolver.h) y [`sudokuSolver.cpp`](src/sudokuSolver.cpp). Este módulo convierte un sudoku en un problema SAT utilizando el formato DIMACS, lo resuelve utilizando el SAT Solver implementado, y luego traduce la solución de vuelta al formato de sudoku.

El proceso incluye:
- La conversión de un sudoku a cláusulas en formato DIMACS.
- La resolución del problema SAT utilizando el SAT Solver.
- La interpretación del modelo resultante para reconstruir la solución del sudoku.

### Nota

Durante la ejecución, el código genera un archivo llamado `sudoku_dimacs_*.cnf` que contiene la representación del sudoku que se está procesando en formato CNF.

### Limitaciones

Aunque en teoría el `dpllSolver` debería resolver los sudokus, se requiere un tiempo considerable para obtener una solución. Durante las pruebas realizadas, incluso con 2 horas de cómputo, no se logró resolver sudokus como "Cheese" o "Fata Morgana". Por lo tanto, esta parte del proyecto no pudo ser completamente testeada.

