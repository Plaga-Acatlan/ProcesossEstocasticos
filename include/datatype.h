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


typedef struct {
    int depth;
    int *rows;
    int *cols;
    double ***data;
} Matrix3D;

// Funciones para Matrix3D
Matrix3D* create_matrix3d(int depth, int *rows, int *cols);
void free_matrix3d(Matrix3D *mat3d);
void print_matrix3d(Matrix3D *mat3d);

// Variables globales
extern Matrix *g_input_matrix;
extern Matrix *g_output_matrix;
extern Matrix3D *g_matrix_3d;
extern int NUM_ESTADOS;
extern int NUM_POLITICAS;
extern int NUM_DECISIONES;
extern char TIPO[3];
extern int rows[];
extern int cols[];
extern double flat_data[];

#endif // DATATYPE_H