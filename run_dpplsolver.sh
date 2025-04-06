#!/bin/bash

# Pedir al usuario la ruta de la carpeta
read -p "Introduce la dirección de la carpeta: " folder_path

# Verificar si la carpeta existe
if [ ! -d "$folder_path" ]; then
    echo "Error: La carpeta no existe o no es válida."
    exit 1
fi

# Iterar sobre los archivos .cnf en la carpeta
for file in "$folder_path"/*.cnf; do
    # Verificar si hay archivos .cnf en la carpeta
    if [ ! -e "$file" ]; then
        echo "No se encontraron archivos .cnf en la carpeta."
        exit 1
    fi

    echo "Procesando $file..."
    ./dpllSolver "$file"
done