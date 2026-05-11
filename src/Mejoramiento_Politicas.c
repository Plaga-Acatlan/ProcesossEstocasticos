#include "Mejoramiento_Politicas.h"

Matrix *Mejoramiento_Politicas(int politica_inicial){
    politica_inicial--;
    Matrix *iteraciones = create_matrix(1, NUM_ESTADOS);
    Vector *V = create_vector(NUM_ESTADOS);
    Vector *V_last = create_vector(NUM_ESTADOS);
    int iteracion_count = 0;

    FOREACH_INDEX(i, NUM_ESTADOS){
        V->data[i] = g_politicas->data[politica_inicial][i];
        iteraciones->data[iteracion_count][i] = V->data[i];
    }
    
    while(!is_vector_equals(V, V_last)){
        // Guardar iteración anterior
        Matrix *A = create_matrix(NUM_ESTADOS+1, NUM_ESTADOS+1);
        Vector *b = create_vector(NUM_ESTADOS+1);

        FOREACH_INDEX(i, NUM_ESTADOS){
            V_last->data[i] = V->data[i];
            V->data[i] = 0;
        }

        for(int i=0;i < NUM_ESTADOS; i++) {
            int k = V_last->data[i]-1; // Política actual para el estado i
            
            // Sistema a resolver:
            // V[i] = C[i][k] + sum(P[i][j][k] * V_last[j]) para j=1 a NUM_ESTADOS
            
            b->data[i] = g_costos->data[i][k]; // C[i][k]
            for(int j = 0; j < NUM_ESTADOS; j++){
                if(j==NUM_ESTADOS-1) A->data[i][j] = 0; 
                else A->data[i][j] = (i == j) ? 1-g_transiciones_3d->data[k][i][j] : -g_transiciones_3d->data[k][i][j]; // Coeficientes para V[j]
            }
            
            A->data[i][NUM_ESTADOS] = 1;
        }
        
        for(int j=0; j < NUM_ESTADOS+1; j++){
            A->data[NUM_ESTADOS][j] = 0; // Restricción de suma de probabilidades
        }
        A->data[NUM_ESTADOS][NUM_ESTADOS-1] = 1;
        b->data[NUM_ESTADOS] = 0;

        Vector *solution= create_vector(NUM_ESTADOS+1);
        gauss_jordan(A, b, solution);
        
        // Actualizar V con la solución obtenida
        for(int i=0; i < NUM_ESTADOS; i++){
            double value = DBL_MAX; 
            int k_index = -1;
            for(int k=0; k < NUM_DECISIONES; k++){
                if(g_transiciones_3d->data[k][i][0] < 0) continue; 
                // Si hay transición desde estado i con decisión k
                double costo_ik = g_costos->data[i][k];
                double valor_ij = 0;
                    for(int j=0; j < NUM_ESTADOS; j++){
                        valor_ij += g_transiciones_3d->data[k][i][j] * solution->data[j];
                    }
                valor_ij += costo_ik - solution->data[i]; // C[i][k] + sum(P[i][j][k] * V_last[j]) 
                if (valor_ij<value){
                    value = valor_ij;
                    k_index = k+1;
                }
            }
            V->data[i] = k_index;
        }
        
        /* Resultados debug:
        log_message("LOG", "Resultados de la iteración %i", iteracion_count++);
        print_vector(V);
        print_vector(V_last);
        */

        iteracion_count++;
        double **new_data = (double**)realloc(iteraciones->data, 
                                              (iteraciones->rows + 1) * sizeof(double*));
        if (!new_data) {
            log_error("iteraciones", "Fallo de memoria al expandir", -1);
            break;
        }
        iteraciones->data = new_data;
        // Inicializar nueva fila
        iteraciones->data[iteraciones->rows] = (double*)calloc(NUM_ESTADOS, sizeof(double));
        iteraciones->rows++;
        FOREACH_INDEX(i, NUM_ESTADOS) {
            iteraciones->data[iteracion_count][i] = V->data[i];
        }

        free_matrix(A);
        free_vector(b);
        free_vector(solution);
    }
    
    FOREACH_INDEX(i, NUM_ESTADOS){
        iteraciones->data[iteracion_count][i] = V->data[i];
    }
    
    free_vector(V);
    free_vector(V_last);
    return iteraciones;
}