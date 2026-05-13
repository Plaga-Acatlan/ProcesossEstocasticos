#include "setup.h"
#include "utils.h"
#include "datatype.h"
#include <string.h>

#ifndef EEP_H
#define EEP_H

// Función para crear la matriz de transiciones para una política específica
Matrix *create_political_matrix(int politica_index);
// Función principal para resolver el EEP y retornar la matriz de resultados
Matrix *solve_eep();
// Función para encontrar la política óptima
int return_optimal(Matrix *EEP_Solution);
#endif // EEP_H