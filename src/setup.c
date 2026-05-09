#include "setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variables globales
Matrix *g_input_matrix = NULL;
Matrix *g_output_matrix = NULL;
Matrix3D *g_matrix_3d = NULL;
Vector *g_weights = NULL;

// Configuración por defecto
int NUM_ESTADOS = 2;
int NUM_POLITICAS = 4;
int NUM_DECISIONES = 2;
char TIPO[4] = "min"; 

int rows[] = {2, 2};
int cols[] = {2, 2};
double flat_data[] = {
    0.6, 0.4,
    0.6, 0.4,

    0.4, 0.6,
    0.5, 0.5
};

// === Función para actualizar configuración ===
int set_configuration(int num_estados, int num_politicas, int num_decisiones, const char *tipo) {
    if (num_estados <= 0 || num_politicas <= 0 || num_decisiones <= 0 || !tipo) {
        return -1; // Configuración inválida
    }
    NUM_ESTADOS = num_estados;
    NUM_POLITICAS = num_politicas;
    NUM_DECISIONES = num_decisiones;
    strncpy(TIPO, tipo, sizeof(TIPO) - 1);
    TIPO[sizeof(TIPO) - 1] = '\0'; // Asegurar terminación nula
    return 0;
}

// === Inicialización de matrices globales ===
void init_global_matrices(void) {
    // Liberar matrices existentes si ya fueron creadas (evita memory leaks)
    free_global_matrices();
    
    // Crear Matrix3D con dimensiones actuales
    g_matrix_3d= create_matrix3d(NUM_DECISIONES, rows, cols);
    if (!g_matrix_3d) {
        fprintf(stderr, "Error: no se pudo crear g_matrix_3d\n");
        return;
    }
    
    // Llenar con datos
    if (fill_matrix3d_from_array(g_matrix_3d, flat_data) != 0) {
        fprintf(stderr, "Error: no se pudo llenar g_matrix_3d\n");
        free_matrix3d(g_matrix_3d);
        g_matrix_3d = NULL;
    }
}
