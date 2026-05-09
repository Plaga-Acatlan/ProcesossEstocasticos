#include "tool.h"
#include "datatype.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Implementación de Gauss-Jordan
int gauss_jordan(Matrix *A, Vector *b, Vector *x) {
    if (!A || !b || !x || A->rows != A->cols || A->rows != b->size || A->rows != x->size) {
        return -1; // Dimensiones incompatibles
    }
    int n = A->rows;

    // Crear matriz aumentada
    Matrix *aug = create_matrix(n, n + 1);
    if (!aug) return -1;

    // Copiar A y b a la matriz aumentada
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            aug->data[i][j] = A->data[i][j];
        }
        aug->data[i][n] = b->data[i];
    }

    // Gauss-Jordan
    for (int p = 0; p < n; p++) {
        // Encontrar pivote
        int max = p;
        for (int i = p + 1; i < n; i++) {
            if (fabs(aug->data[i][p]) > fabs(aug->data[max][p])) {
                max = i;
            }
        }
        // Intercambiar filas
        if (p != max) {
            double *temp = aug->data[p];
            aug->data[p] = aug->data[max];
            aug->data[max] = temp;
        }

        // Verificar si pivote es cero
        if (fabs(aug->data[p][p]) < 1e-10) {
            free_matrix(aug);
            return -1; // Matriz singular
        }

        // Hacer pivote 1
        double pivot = aug->data[p][p];
        for (int j = 0; j <= n; j++) {
            aug->data[p][j] /= pivot;
        }

        // Eliminar otras filas
        for (int i = 0; i < n; i++) {
            if (i != p) {
                double factor = aug->data[i][p];
                for (int j = 0; j <= n; j++) {
                    aug->data[i][j] -= factor * aug->data[p][j];
                }
            }
        }
    }

    // Extraer solución
    for (int i = 0; i < n; i++) {
        x->data[i] = aug->data[i][n];
    }

    free_matrix(aug);
    return 0;
}

int fill_matrix3d_from_array(Matrix3D *mat3d, const double *flat_data) {
    if (!mat3d || !flat_data) return -1;
    
    int offset = 0; 
    
    for (int d = 0; d < mat3d->depth; d++) {
        int rows = mat3d->rows[d];
        int cols = mat3d->cols[d];
        int layer_size = rows * cols;
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                mat3d->data[d][i][j] = flat_data[offset++];
            }
        }
    }
    
    return 0;
}