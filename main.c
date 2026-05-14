#include "solving_systems.h"
#include <locale.h>

void load_default_data() {
    // Datos personalizados para el ejemplo
    int estados = 4;
    int decisiones = 3;
    int politicas = 6;
    char tipo[4] = "min";
    free_global_matrices();

    set_configuration(estados, politicas, decisiones, tipo);
    double mis_transiciones[] = {
        // Decisión 0
        0.0, 7.0/8.0, 1.0/16.0, 1.0/16.0,
        0.0, 3.0/4.0, 1.0/8.0, 1.0/8.0,
        0.0, 0.0, 1.0/2.0, 1.0/2.0,
        -1.0, -1.0, -1.0, -1.0,
        // Decisión 1
        -1.0, -1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0, -1.0,
        0.0, 1.0, 0.0, 0.0,
        -1.0, -1.0, -1.0, -1.0,
        // Decisión 2
        -1.0, -1.0, -1.0, -1.0,
        1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0
    };

    double mis_costos[] = {
        0.0, 0.0, 0.0,
        1000.0, 0.0, 6000.0,
        3000.0, 4000.0, 6000.0,
        0.0, 0.0, 6000.0
    };

    int mis_politicas[] = {
        1, 1, 1, 3,
        1, 1, 2, 3,
        1, 1, 3, 3,
        1, 3, 1, 3,
        1, 3, 2, 3,
        1, 3, 3, 3
    };

    init_with_custom_data(mis_transiciones, mis_costos, mis_politicas);
}

int main() {
    // Configuración inicial (puede ser modificada según necesidades)
    if (setlocale(LC_ALL, "es_MX.UTF-8") == NULL) {
        setlocale(LC_ALL, "Spanish_Mexico.1252"); 
    }
    //init_global_matrices();
    load_default_data();
    Matrix *resultados = AS(10,2,1);
    
    print_matrix(resultados);

    free_global_matrices();
    free_matrix(resultados);

    return 0;
}