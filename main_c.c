#include "datatype.h"
#include "tool.h"
#include <stdio.h>

int main() {
    printf("=== Sistema de Ecuaciones Lineales - Metodo Gauss-Jordan ===\n\n");

    // Ejemplo 1: Sistema 2x2
    // 2x + y = 3
    // x - y = 0
    printf("Ejemplo 1: Sistema 2x2\n");
    printf("Ecuaciones:\n");
    printf("  2x + y = 3\n");
    printf("  x - y = 0\n\n");

    Matrix *A1 = create_matrix(2, 2);
    Vector *b1 = create_vector(2);
    Vector *x1 = create_vector(2);

    // Matriz de coeficientes
    A1->data[0][0] = 2;  A1->data[0][1] = 1;
    A1->data[1][0] = 1;  A1->data[1][1] = -1;

    // Vector de términos independientes
    b1->data[0] = 3;
    b1->data[1] = 0;

    printf("Matriz A:\n");
    print_matrix(A1);
    printf("Vector b:\n");
    print_vector(b1);

    if (gauss_jordan(A1, b1, x1) == 0) {
        printf("Solución x:\n");
        print_vector(x1);
        printf("Verificación: 2*%.2f + %.2f = %.2f (debe ser 3.00)\n\n", 
               x1->data[0], x1->data[1], 2*x1->data[0] + x1->data[1]);
    } else {
        printf("Error: Matriz singular o dimensiones incorrectas.\n\n");
    }

    free_matrix(A1);
    free_vector(b1);
    free_vector(x1);

    // Ejemplo 2: Sistema 3x3
    // x + y + z = 6
    // 2x + y - z = 3
    // x + 2y + z = 8
    printf("Ejemplo 2: Sistema 3x3\n");
    printf("Ecuaciones:\n");
    printf("  x + y + z = 6\n");
    printf("  2x + y - z = 3\n");
    printf("  x + 2y + z = 8\n\n");

    Matrix *A2 = create_matrix(3, 3);
    Vector *b2 = create_vector(3);
    Vector *x2 = create_vector(3);

    // Matriz de coeficientes
    A2->data[0][0] = 1;  A2->data[0][1] = 1;  A2->data[0][2] = 1;
    A2->data[1][0] = 2;  A2->data[1][1] = 1;  A2->data[1][2] = -1;
    A2->data[2][0] = 1;  A2->data[2][1] = 2;  A2->data[2][2] = 1;

    // Vector de términos independientes
    b2->data[0] = 6;
    b2->data[1] = 3;
    b2->data[2] = 8;

    printf("Matriz A:\n");
    print_matrix(A2);
    printf("Vector b:\n");
    print_vector(b2);

    if (gauss_jordan(A2, b2, x2) == 0) {
        printf("Solución x:\n");
        print_vector(x2);
        printf("Verificación: %.2f + %.2f + %.2f = %.2f (debe ser 6.00)\n\n",
               x2->data[0], x2->data[1], x2->data[2], 
               x2->data[0] + x2->data[1] + x2->data[2]);
    } else {
        printf("Error: Matriz singular o dimensiones incorrectas.\n\n");
    }

    free_matrix(A2);
    free_vector(b2);
    free_vector(x2);

    printf("=== Fin del programa ===\n");
    return 0;
}