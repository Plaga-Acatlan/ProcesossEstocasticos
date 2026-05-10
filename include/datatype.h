#ifndef DATATYPE_H
#define DATATYPE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <float.h>

typedef struct {
    int rows;
    int cols;
    double **data;
} Matrix;

// Creación / Liberación / Visualización
Matrix* create_matrix(int rows, int cols);
void free_matrix(Matrix *mat);
void print_matrix(Matrix *mat);

typedef struct {
    int size;
    double *data;
} Vector;

// Creación / Liberación / Visualización
Vector* create_vector(int size);
void free_vector(Vector *vec);
void print_vector(Vector *vec);

typedef struct {
    int depth;
    int *rows;
    int *cols;
    double ***data;
} Matrix3D;

// Creación / Liberación / Visualización
Matrix3D* create_matrix3d(int depth, int *rows, int *cols);
void free_matrix3d(Matrix3D *mat3d);
void print_matrix3d(Matrix3D *mat3d);

int fill_matrix3d_from_array(Matrix3D *mat3d, const double *flat_data);
int flatten_matrix3d(const Matrix3D *mat3d, double *output);
int get_matrix3d_total_size(const Matrix3D *mat3d);
bool is_valid_matrix(const Matrix *mat);

#endif // DATATYPE_H