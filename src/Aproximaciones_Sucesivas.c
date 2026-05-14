#include "Aproximaciones_Sucesivas.h"

Matrix *AS(double tolerancia, int max_iteraciones, double alpha){
    int n = 1;
    double diferencia = DBL_MAX;
    Matrix *iteraciones = create_matrix(1, NUM_ESTADOS);
    Vector *V = create_vector(NUM_ESTADOS);
    Vector *Costos_anterior = create_vector(NUM_ESTADOS);
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
                if (g_transiciones_3d->data[k][i][0] < 0) continue;
                double valor_ij = g_costos->data[i][k];
                double probabilidades = 0;

                //printf("K = %d, V_%d^%d = min {", k+1, i, n);
                for (int j = 0; n>1 && j < NUM_ESTADOS; j++){
                    probabilidades += g_transiciones_3d->data[k][i][j]*Costos_anterior->data[j];
                    //printf("%s%.4f*%.4f", (j==0) ? "" : " + ", g_transiciones_3d->data[k][i][j], Costos_anterior->data[j]);
                }
                //printf("} + %.4f\n", g_costos->data[i][k]);
                valor_ij += alpha * probabilidades;                

                
                if ((!maximizar && valor_ij<value) || (maximizar && valor_ij>value)) {
                    index = k;
                    value = valor_ij;
                }
            }
            diferencia = max(diferencia, (n>1) ? abs_double(iteraciones->data[n-1][i] - Costos->data[i]) : Costos->data[i]);
            V->data[i] = index+1;
            Costos->data[i] = value;
        }

        FOREACH_INDEX(i, NUM_ESTADOS) {
            iteraciones->data[n-1][i] = V->data[i];
            Costos_anterior->data[i] = Costos->data[i];
        }

        if(diferencia <= tolerancia || n >= max_iteraciones){ 
            break;
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
        

        n++;
    }

    free_vector(V);
    free_vector(Costos);
    free_vector(Costos_anterior);

    return iteraciones;
}