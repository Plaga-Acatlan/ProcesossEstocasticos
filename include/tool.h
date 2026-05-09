#ifndef TOOL_H
#define TOOL_H

#include "datatype.h"

// Función para resolver un sistema de ecuaciones lineales usando Gauss-Jordan
// A es la matriz de coeficientes, b es el vector de términos independientes
// x es el vector solución (debe estar preasignado)
// Retorna 0 si éxito, -1 si error (matriz singular, etc.)
int gauss_jordan(Matrix *A, Vector *b, Vector *x);

int set_configuration(int num_estados, int num_politicas, int num_decisiones, const char *tipo);

int fill_matrix3d_from_array(Matrix3D *mat3d, const double *flat_data);

#endif // TOOL_H