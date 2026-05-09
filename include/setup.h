#ifndef SETUP_H
#define SETUP_H

#include "datatype.h"
#include "tool.h"
#include <string.h>

// === VARIABLES GLOBALES===
extern Matrix *g_input_matrix;
extern Matrix *g_output_matrix;
extern Matrix3D *g_matrix_3d;
extern Vector *g_weights;

// === Parámetros de configuración globales ===
extern int NUM_ESTADOS;
extern int NUM_POLITICAS;
extern int NUM_DECISIONES;
extern char TIPO[3];

// === Arrays de dimensiones y datos por defecto ===
extern int rows[];
extern int cols[];
extern double flat_data[];

// === Funciones de configuración e inicialización ===
int set_configuration(int num_estados, int num_politicas, int num_decisiones, const char *tipo);
void init_global_matrices(void);
void free_global_matrices(void);

#endif // SETUP_H