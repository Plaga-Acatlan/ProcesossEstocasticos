#include "datatype.h"
#include <string.h>  // Para memcpy en flatten

// Matriz 2D

Matrix* create_matrix(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return NULL;
    
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
        mat->data[i] = (double*)calloc(cols, sizeof(double)); 
        if (!mat->data[i]) {
            for (int j = 0; j < i; j++) free(mat->data[j]);
            free(mat->data);
            free(mat);
            return NULL;
        }
    }
    return mat;
}

void free_matrix(Matrix *mat) {
    if (!mat) return;
    if (mat->data) {
        for (int i = 0; i < mat->rows; i++) {
            if (mat->data[i]) free(mat->data[i]);
        }
        free(mat->data);
    }
    free(mat);
}

void print_matrix(Matrix *mat) {
    if (!mat || !mat->data) return;
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            printf("%.2f ", mat->data[i][j]);
        }
        printf("\n");
    }
}

// Vector

Vector* create_vector(int size) {
    if (size <= 0) return NULL;
    
    Vector *vec = (Vector*)malloc(sizeof(Vector));
    if (!vec) return NULL;
    
    vec->size = size;
    vec->data = (double*)calloc(size, sizeof(double));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void free_vector(Vector *vec) {
    if (!vec) return;
    if (vec->data) free(vec->data);
    free(vec);
}

void print_vector(Vector *vec) {
    if (!vec || !vec->data) return;
    for (int i = 0; i < vec->size; i++) {
        printf("%.2f ", vec->data[i]);
    }
    printf("\n");
}

// Matriz 3D
Matrix3D* create_matrix3d(int depth, int *rows, int *cols) {
    if (depth <= 0 || !rows || !cols) return NULL;
    
    Matrix3D *mat3d = (Matrix3D*)calloc(1, sizeof(Matrix3D));  // calloc para NULL inicial
    if (!mat3d) return NULL;
    
    mat3d->depth = depth;
    
    // Reservar y copiar arrays de dimensiones
    mat3d->rows = (int*)malloc(depth * sizeof(int));
    mat3d->cols = (int*)malloc(depth * sizeof(int));
    if (!mat3d->rows || !mat3d->cols) {
        free(mat3d->rows); free(mat3d->cols); free(mat3d);
        return NULL;
    }
    memcpy(mat3d->rows, rows, depth * sizeof(int));
    memcpy(mat3d->cols, cols, depth * sizeof(int));
    
    // Reservar arreglo de punteros por capa
    mat3d->data = (double***)malloc(depth * sizeof(double**));
    if (!mat3d->data) {
        free(mat3d->rows); free(mat3d->cols); free(mat3d);
        return NULL;
    }
    
    // Crear cada capa 2D
    for (int d = 0; d < depth; d++) {
        int r = rows[d], c = cols[d];
        
        mat3d->data[d] = (double**)malloc(r * sizeof(double*));
        if (!mat3d->data[d]) goto cleanup_error;
        
        for (int i = 0; i < r; i++) {
            mat3d->data[d][i] = (double*)calloc(c, sizeof(double));
            if (!mat3d->data[d][i]) goto cleanup_error;
        }
    }
    
    return mat3d;

cleanup_error:
    // Liberación centralizada ante error en cualquier capa
    for (int d = 0; d < depth; d++) {
        if (mat3d->data && mat3d->data[d]) {
            for (int i = 0; i < rows[d]; i++) {
                if (mat3d->data[d][i]) free(mat3d->data[d][i]);
            }
            free(mat3d->data[d]);
        }
    }
    free(mat3d->data);
    free(mat3d->rows);
    free(mat3d->cols);
    free(mat3d);
    return NULL;
}

void free_matrix3d(Matrix3D *mat3d) {
    if (!mat3d) return;
    
    if (mat3d->data) {
        for (int d = 0; d < mat3d->depth; d++) {
            if (mat3d->data[d]) {
                for (int i = 0; i < mat3d->rows[d]; i++) {
                    if (mat3d->data[d][i]) free(mat3d->data[d][i]);
                }
                free(mat3d->data[d]);
            }
        }
        free(mat3d->data);
    }
    
    free(mat3d->rows);
    free(mat3d->cols);
    free(mat3d);
}

void print_matrix3d(Matrix3D *mat3d) {
    if (!mat3d || !mat3d->data) return;
    
    printf("=== Matrix3D [depth=%d] ===\n", mat3d->depth);
    for (int d = 0; d < mat3d->depth; d++) {
        printf("\n--- Capa %d (%dx%d) ---\n", d, mat3d->rows[d], mat3d->cols[d]);
        for (int i = 0; i < mat3d->rows[d]; i++) {
            for (int j = 0; j < mat3d->cols[d]; j++) {
                printf("%.2f ", mat3d->data[d][i][j]);
            }
            printf("\n");
        }
    }
    printf("=== Fin Matrix3D ===\n");
}


int fill_matrix3d_from_array(Matrix3D *mat3d, const double *flat_data) {
    if (!mat3d || !flat_data) return -1;
    
    int offset = 0;
    for (int d = 0; d < mat3d->depth; d++) {
        int r = mat3d->rows[d], c = mat3d->cols[d];
        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
                mat3d->data[d][i][j] = flat_data[offset++];
            }
        }
    }
    return 0;
}

int flatten_matrix3d(const Matrix3D *mat3d, double *output) {
    if (!mat3d || !output) return -1;
    
    int offset = 0;
    for (int d = 0; d < mat3d->depth; d++) {
        for (int i = 0; i < mat3d->rows[d]; i++) {
            for (int j = 0; j < mat3d->cols[d]; j++) {
                output[offset++] = mat3d->data[d][i][j];
            }
        }
    }
    return 0;
}

int get_matrix3d_total_size(const Matrix3D *mat3d) {
    if (!mat3d) return 0;
    int total = 0;
    for (int d = 0; d < mat3d->depth; d++) {
        total += mat3d->rows[d] * mat3d->cols[d];
    }
    return total;
}

bool is_valid_matrix(const Matrix *mat) {
    if (!mat || mat->rows <= 0 || mat->cols <= 0 || !mat->data) return false;
    for (int i = 0; i < mat->rows; i++) {
        if (!mat->data[i]) return false;
    }
    return true;
}
