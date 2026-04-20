#ifndef DATATYPE_H
#define DATATYPE_H

#include <stdlib.h>
#include <stdio.h>

// Estructura para representar una matriz
typedef struct {
    int rows;
    int cols;
    double **data;
} Matrix;

// Función para crear una matriz
Matrix* create_matrix(int rows, int cols);

// Función para liberar memoria de una matriz
void free_matrix(Matrix *mat);

// Función para imprimir una matriz
void print_matrix(Matrix *mat);

// Estructura para representar un vector
typedef struct {
    int size;
    double *data;
} Vector;

// Función para crear un vector
Vector* create_vector(int size);

// Función para liberar memoria de un vector
void free_vector(Vector *vec);

// Función para imprimir un vector
void print_vector(Vector *vec);

#endif // DATATYPE_H