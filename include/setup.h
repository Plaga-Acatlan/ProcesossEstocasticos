#ifndef SETUP_H
#define SETUP_H

#include "datatype.h"
#include <stdbool.h>

// VARIABLES GLOBALES
extern Matrix *g_input_matrix;
extern Matrix *g_output_matrix;
extern Matrix3D *g_transiciones_3d;
extern Vector *g_weights;
extern Matrix *g_costos;
extern Matrix *g_politicas;
//extern Matrix *g_accesibles;

// PARÁMETROS DE CONFIGURACIÓN GLOBAL
extern int NUM_ESTADOS;
extern int NUM_POLITICAS;
extern int NUM_DECISIONES;
extern char TIPO[4];
extern bool maximizar;

// FUNCIONES PÚBLICAS DE SETUP
// Configura los parámetros del sistema.
int set_configuration(int num_estados, int num_politicas, int num_decisiones, const char *tipo);

// Inicializa todas las estructuras globales según la configuración actual.
 void init_global_matrices(void);
 void init_with_custom_data(const double *transiciones_flat, const double *costos, const int *politicas);

// Libera toda la memoria gestionada por setup y reinicia el estado interno.
void free_global_matrices(void);

// Verifica si el sistema ya fue inicializado.
bool is_setup_initialized(void);

#endif // SETUP_H