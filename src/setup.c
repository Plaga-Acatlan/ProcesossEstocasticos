#include "setup.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// DEFINICIÓN DE VARIABLES GLOBALES
Matrix *g_input_matrix = NULL;
Matrix *g_output_matrix = NULL;
Matrix3D *g_transiciones_3d = NULL;
Vector *g_weights = NULL;
Matrix *g_costos = NULL;
Matrix *g_politicas = NULL;

// CONFIGURACIÓN POR DEFECTO
int NUM_ESTADOS = 2;
int NUM_POLITICAS = 4;
int NUM_DECISIONES = 2;
char TIPO[4] = "min";


// Datos internos por defecto
static double default_transiciones[] = {
    // Capa 0
    0.6, 0.4,
    0.6, 0.4,
    // Capa 1
    0.4, 0.6,
    0.5, 0.5
};

static double DEFAULT_COSTOS[2][2] = {
    {0.0, 0.0},
    {1200.0, 1200.0}
};

static int DEFAULT_POLITICAS[4][2] = {
    {1, 1},
    {1, 2},
    {2, 1},
    {2, 2} 
};

// Estado interno de inicialización
static bool g_initialized = false;

// IMPLEMENTACIÓN DE FUNCIONES
bool is_setup_initialized(void) {
    return g_initialized;
}

int set_configuration(int num_estados, int num_politicas, int num_decisiones, const char *tipo) {
    if (num_estados <= 0 || num_politicas <= 0 || num_decisiones <= 0 || !tipo) {
        log_error("set_configuration", "Parámetros inválidos recibidos", -1);
        return -1;
    }

    NUM_ESTADOS = num_estados;
    NUM_POLITICAS = num_politicas;
    NUM_DECISIONES = num_decisiones;

    // Copia segura con terminación garantizada
    strncpy(TIPO, tipo, sizeof(TIPO) - 1);
    TIPO[sizeof(TIPO) - 1] = '\0';

    log_message("INFO", "Configuración actualizada: E=%d | P=%d | D=%d | Tipo=%s",
                NUM_ESTADOS, NUM_POLITICAS, NUM_DECISIONES, TIPO);
    return 0;
}

void init_global_matrices(void) {
    if (g_initialized) {
        log_message("WARN", "Re-inicializando sistema...");
        free_global_matrices();
    }

    log_message("INFO", "Inicializando estructuras...");

    // 1. Transiciones: [decisión][estado][estado_siguiente]
    int rows[NUM_DECISIONES], cols[NUM_DECISIONES];
    for (int d = 0; d < NUM_DECISIONES; d++) {
        rows[d] = NUM_ESTADOS;
        cols[d] = NUM_ESTADOS;
    }
    
    g_transiciones_3d = create_matrix3d(NUM_DECISIONES, rows, cols);
    if (!g_transiciones_3d) {
        log_error("init_global_matrices", "Fallo al crear tensor de transiciones", -1);
        goto init_error;
    }
    
    if (fill_matrix3d_from_array(g_transiciones_3d, default_transiciones) != 0) {
        log_error("init_global_matrices", "Fallo al llenar tensor de transiciones", -1);
        goto init_error;
    }

    // 2. Matriz de Costos: [estado][decisión]
    g_costos = create_matrix(NUM_ESTADOS, NUM_DECISIONES);
    if (!g_costos) {
        log_error("init_global_matrices", "Fallo al crear matriz de costos", -1);
        goto init_error;
    }
    
    // Precargar costos
    for (int s = 0; s < NUM_ESTADOS; s++) {
        for (int a = 0; a < NUM_DECISIONES; a++) {
            g_costos->data[s][a] = DEFAULT_COSTOS[s][a];
        }
    }

    // 3. Matriz de Políticas: [política][estado] → decisión
    g_politicas = create_matrix(NUM_POLITICAS, NUM_ESTADOS);
    if (!g_politicas) {
        log_error("init_global_matrices", "Fallo al crear matriz de políticas", -1);
        goto init_error;
    }
    
    // Precargar políticas
    for (int p = 0; p < NUM_POLITICAS; p++) {
        for (int s = 0; s < NUM_ESTADOS; s++) {
            g_politicas->data[p][s] = DEFAULT_POLITICAS[p][s];
        }
    }

    g_initialized = true;
    log_message("INFO", "Inicialización completada: E=%d, D=%d, P=%d", 
                NUM_ESTADOS, NUM_DECISIONES, NUM_POLITICAS);
    return;

init_error:
    log_error("init_global_matrices", "Fallo crítico de inicialización", -1);
    free_global_matrices();
}

void init_with_custom_data(
    const double *transiciones_flat,
    const double *costos,
    const int *politicas)
{
    // Limpieza previa por seguridad
    if (g_initialized) free_global_matrices();
    
    log_message("INFO", "Inicializando con datos personalizados...");
    
    // 1. Crear transiciones
    int dims[NUM_DECISIONES];
    for (int d = 0; d < NUM_DECISIONES; d++) dims[d] = NUM_ESTADOS;
    
    g_transiciones_3d = create_matrix3d(NUM_DECISIONES, dims, dims);
    if (!g_transiciones_3d) goto error;
    fill_matrix3d_from_array(g_transiciones_3d, transiciones_flat);
    
    // 2. Crear y llenar matriz de costos
    g_costos = create_matrix(NUM_ESTADOS, NUM_DECISIONES);
    if (!g_costos) goto error;
    for (int s = 0; s < NUM_ESTADOS; s++)
        for (int a = 0; a < NUM_DECISIONES; a++)
            g_costos->data[s][a] = costos[s * NUM_DECISIONES + a];
    
    // 3. Crear y llenar matriz de políticas
    g_politicas = create_matrix(NUM_POLITICAS, NUM_ESTADOS);
    if (!g_politicas) goto error;
    for (int p = 0; p < NUM_POLITICAS; p++)
        for (int s = 0; s < NUM_ESTADOS; s++)
            g_politicas->data[p][s] = (double)politicas[p * NUM_ESTADOS + s];
    
    
    g_initialized = true;
    log_message("INFO", "Inicialización personalizada completada");
    return;
    
error:
    log_error("init_with_custom_data", "Fallo de memoria", -1);
    free_global_matrices();
}

void free_global_matrices(void) {
    if (!g_initialized && !g_transiciones_3d && !g_input_matrix && !g_output_matrix && !g_weights) {
        return; // Nada que liberar
    }

    log_message("INFO", "Liberando memoria de estructuras globales...");

    if (g_input_matrix)  { free_matrix(g_input_matrix);  g_input_matrix  = NULL; }
    if (g_output_matrix) { free_matrix(g_output_matrix); g_output_matrix = NULL; }
    if (g_transiciones_3d) { free_matrix3d(g_transiciones_3d); g_transiciones_3d = NULL; }
    if (g_weights)       { free_vector(g_weights);       g_weights       = NULL; }
    if (g_costos) { free_matrix(g_costos); g_costos = NULL; }
    if (g_politicas) { free_matrix(g_politicas); g_politicas = NULL; }

    g_initialized = false;
    log_message("INFO", "Memoria liberada correctamente.");
}