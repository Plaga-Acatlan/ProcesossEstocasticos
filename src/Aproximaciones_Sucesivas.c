#include "Aproximaciones_Sucesivas.h"

Matrix *AS(double tolerancia, int max_iteraciones, double alpha){
    int n = 1;
    double diferencia = DBL_MAX;
    Matrix *iteraciones = create_matrix(1, NUM_ESTADOS);
    Vector *V = create_vector(NUM_ESTADOS);
    Vector *Costos = create_vector(NUM_ESTADOS);

    FOREACH_INDEX(i, NUM_ESTADOS){
        Costos->data[i] = DBL_MAX;
        iteraciones->data[0][i] = 0;
    }
    
    while (n <= max_iteraciones && diferencia>tolerancia) {
        diferencia = DBL_MIN;
        for (int i = 0; i < NUM_ESTADOS; i++){
            int index = -1; double value = (maximizar) ? DBL_MIN : DBL_MAX;

            for (int k=0; k < NUM_DECISIONES; k++){
                double valor_ij = g_costos->data[i][k];
                double probabilidades = 0;
                if (g_transiciones_3d->data[k][i][0] < 0) continue;

                for (int j = 0; n!=1 && j < NUM_ESTADOS; j++){
                    probabilidades += g_transiciones_3d->data[k][i][j]*Costos->data[j];
                }
                valor_ij += alpha * probabilidades;
                printf("Para k = %d, V_%d^%d = %.4f\t", k+1, i, n, valor_ij);
                printf(" Costo[%d][%d] = %.4f | P[%i][0](%d) = | probas = %.4f\n", i, k+1, g_costos->data[i][k], i, k+1, probabilidades);

                
                if ((!maximizar && valor_ij<value) || (maximizar && valor_ij>value)) {
                    index = k;
                    value = valor_ij;
                }
            }
            diferencia = max(diferencia, (n!=1) ? abs_double(iteraciones->data[n-1][i] - Costos->data[i]) : Costos->data[i]);
            V->data[i] = index+1;
            Costos->data[i] = value;
        }

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
            iteraciones->data[n-1][i] = V->data[i];
        }
        n++;
    }

    free_vector(V);
    free_vector(Costos);

    return iteraciones;
}