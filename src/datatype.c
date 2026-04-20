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