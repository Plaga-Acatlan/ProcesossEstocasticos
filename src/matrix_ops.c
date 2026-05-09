// matrix_ops.c
#include <stdlib.h>

// Para Windows descomenta la línea siguiente:
// #define EXPORT __declspec(dllexport)
// #define EXPORT

// Linux/macOS: no necesita decorador
#ifndef EXPORT
#define EXPORT
#endif

EXPORT void sumar_matrices(const double *A, const double *B, double *C, int filas, int columnas) {
    int total = filas * columnas;
    for (int i = 0; i < total; i++) {
        C[i] = A[i] + B[i];
    }
}