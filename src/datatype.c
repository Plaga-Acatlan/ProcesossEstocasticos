#include "datatype.h"

// Implementación de create_matrix
Matrix* create_matrix(int rows, int cols) {
    Matrix *mat = (Matrix*)malloc(sizeof(Matrix));
    if (!mat) return NULL;
    mat->rows = rows;
    mat->cols = cols;
    mat->data = (double**)malloc(rows * sizeof(double*));
    if (!mat->data) {
        free(mat);
        return NULL;
    }
    for (int i = 0; i < rows; i++) {
        mat->data[i] = (double*)malloc(cols * sizeof(double));
        if (!mat->data[i]) {
            for (int j = 0; j < i; j++) free(mat->data[j]);
            free(mat->data);
            free(mat);
            return NULL;
        }
    }
    return mat;
}

// Implementación de free_matrix
void free_matrix(Matrix *mat) {
    if (!mat) return;
    for (int i = 0; i < mat->rows; i++) {
        free(mat->data[i]);
    }
    free(mat->data);
    free(mat);
}

// Implementación de print_matrix
void print_matrix(Matrix *mat) {
    if (!mat) return;
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            printf("%.2f ", mat->data[i][j]);
        }
        printf("\n");
    }
}

// Implementación de create_vector
Vector* create_vector(int size) {
    Vector *vec = (Vector*)malloc(sizeof(Vector));
    if (!vec) return NULL;
    vec->size = size;
    vec->data = (double*)malloc(size * sizeof(double));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

// Implementación de free_vector
void free_vector(Vector *vec) {
    if (!vec) return;
    free(vec->data);
    free(vec);
}

// Implementación de print_vector
void print_vector(Vector *vec) {
    if (!vec) return;
    for (int i = 0; i < vec->size; i++) {
        printf("%.2f ", vec->data[i]);
    }
    printf("\n");
}

Matrix3D* create_matrix3d(int depth, int *rows, int *cols) {
    if (depth <= 0 || !rows || !cols) return NULL;
    
    Matrix3D *mat3d = (Matrix3D*)malloc(sizeof(Matrix3D));
    if (!mat3d) return NULL;
    
    mat3d->depth = depth;
    
    // Reservar arrays para dimensiones
    mat3d->rows = (int*)malloc(depth * sizeof(int));
    mat3d->cols = (int*)malloc(depth * sizeof(int));
    if (!mat3d->rows || !mat3d->cols) {
        free(mat3d->rows); free(mat3d->cols); free(mat3d);
        return NULL;
    }
    
    // Copiar dimensiones
    for (int i = 0; i < depth; i++) {
        mat3d->rows[i] = rows[i];
        mat3d->cols[i] = cols[i];
    }
    
    // Reservar arreglo de punteros para cada capa
    mat3d->data = (double***)malloc(depth * sizeof(double**));
    if (!mat3d->data) {
        free(mat3d->rows); free(mat3d->cols); free(mat3d);
        return NULL;
    }
    
    // Crear cada matriz 2D para cada capa
    for (int i = 0; i < depth; i++) {
        mat3d->data[i] = (double**)malloc(rows[i] * sizeof(double*));
        if (!mat3d->data[i]) {
            // Limpieza en caso de error
            for (int j = 0; j < i; j++) {
                for (int k = 0; k < mat3d->rows[j]; k++) 
                    free(mat3d->data[j][k]);
                free(mat3d->data[j]);
            }
            free(mat3d->data);
            free(mat3d->rows); free(mat3d->cols); free(mat3d);
            return NULL;
        }
        
        for (int j = 0; j < rows[i]; j++) {
            mat3d->data[i][j] = (double*)calloc(cols[i], sizeof(double));
            if (!mat3d->data[i][j]) {
                for (int k = 0; k < j; k++) free(mat3d->data[i][k]);
                free(mat3d->data[i]);
                // Limpieza completa...
                for (int d = 0; d < depth; d++) {
                    if (d < i) {
                        for (int r = 0; r < mat3d->rows[d]; r++) 
                            free(mat3d->data[d][r]);
                        free(mat3d->data[d]);
                    } else if (d == i) {
                        // ya liberado arriba
                    }
                }
                free(mat3d->data);
                free(mat3d->rows); free(mat3d->cols); free(mat3d);
                return NULL;
            }
        }
    }
    
    return mat3d;
}

// Liberar memoria de Matrix3D
void free_matrix3d(Matrix3D *mat3d) {
    if (!mat3d) return;
    
    for (int i = 0; i < mat3d->depth; i++) {
        if (mat3d->data && mat3d->data[i]) {
            for (int j = 0; j < mat3d->rows[i]; j++) {
                free(mat3d->data[i][j]);
            }
            free(mat3d->data[i]);
        }
    }
    
    free(mat3d->data);
    free(mat3d->rows);
    free(mat3d->cols);
    free(mat3d);
}

// Imprimir Matrix3D
void print_matrix3d(Matrix3D *mat3d) {
    if (!mat3d) return;
    
    printf("Matrices: %d\n", mat3d->depth);
    for (int d = 0; d < mat3d->depth; d++) {
        printf("\n--- Capa %d (%dx%d) ---\n", d, mat3d->rows[d], mat3d->cols[d]);
        for (int i = 0; i < mat3d->rows[d]; i++) {
            for (int j = 0; j < mat3d->cols[d]; j++) {
                printf("%.2f ", mat3d->data[d][i][j]);
            }
            printf("\n");
        }
    }
    printf("\n");
}