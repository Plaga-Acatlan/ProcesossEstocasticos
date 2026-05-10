#include "solving_systems.h"
#include <locale.h>

void load_default_data() {
    // Datos personalizados para el ejemplo
    int estados = 2;
    int decisiones = 2;
    int politicas = 4;
    char tipo[4] = "min";

    set_configuration(estados, politicas, decisiones, tipo);

    double mis_transiciones[] = {
        // Decisión 0
        0.6, 0.4,
        0.6, 0.4,
        // Decisión 1
        0.4, 0.6,
        0.5, 0.5
    };

    double mis_costos[] = {
        0.0, 0.0,
        1200.0, 1200.0 
    };

    int mis_politicas[] = {
        1, 1,
        1, 2,
        2, 1,
        2, 2
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
    
    Matrix *resultados = solve_eep();
    print_matrix(resultados);

    free_global_matrices();
    free_matrix(resultados);

    return 0;
}