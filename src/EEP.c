#include "EEP.h"

Matrix *create_political_matrix(int politica_index) {
    if (!g_transiciones_3d || !g_politicas) {
        log_error("EEP_calcular_transiciones", "Punteros de entrada NULL", -1);
        return NULL;
    }

    Matrix *out;
    int rows = NUM_ESTADOS;
    int cols = NUM_ESTADOS;
    out = create_matrix(rows, cols);
    if (!out) {
        log_error("EEP_calcular_transiciones", "Fallo de memoria al crear matriz de salida", -1);
        return NULL;
    }
    FOREACH_INDEX(i, rows){
        int capa = g_politicas->data[politica_index][i]-1;
        int fila = i;
        FOREACH_INDEX(j, cols){
            out->data[fila][j] = g_transiciones_3d->data[capa][fila][j];
        }
    }   
    return out;
}

Matrix *solve_eep(){
    if (!g_transiciones_3d || !g_politicas) {
        log_error("solve_eep", "Punteros de entrada NULL", -1);
        return NULL;
    }
    Matrix *out = create_matrix(NUM_POLITICAS, NUM_ESTADOS + 1);
    if (!out) {
        log_error("solve_eep", "Fallo de memoria al crear matriz 3D de salida", -1);
        return NULL;
    }
    
    FOREACH_INDEX(p, NUM_POLITICAS){
        Matrix *mat_politica = create_political_matrix(p);
        if (!mat_politica) {
            free_matrix(out);
            return NULL; // Error ya logueado dentro de create_political_matrix
        }
        Matrix *A = create_matrix(NUM_ESTADOS, NUM_ESTADOS);
        Vector *b = create_vector(NUM_ESTADOS);
        Vector *x = create_vector(NUM_ESTADOS);
        if (!A || !b || !x) {
            log_error("solve_eep", "Fallo de memoria al crear matrices/vectores para Gauss-Jordan", -1);
            free_matrix(mat_politica);
            free_matrix(A); free_vector(b); free_vector(x);
            free_matrix(out);
            return NULL;
        }
        FOREACH_INDEX(i, NUM_ESTADOS){
            FOREACH_INDEX(j, NUM_ESTADOS){
                if(!i) A->data[i][j] = 1.0;
                else A->data[i][j] = (i == j) ? mat_politica->data[j][i] - 1.0: mat_politica->data[j][i];
            }
            b->data[i] = (!i) ? 1.0 : 0.0;
        }

        if (gauss_jordan(A, b, x) != 0) {
            log_error("solve_eep", "Error al resolver sistema lineal con Gauss-Jordan", -1);
            free_matrix(mat_politica);
            free_matrix(A); free_vector(b); free_vector(x);
            free_matrix(out);
            return NULL;
        }
        // Almacenar la solución en la matriz 3D
        int j = 0;
        FOREACH_INDEX(i, NUM_ESTADOS){ 
            int politica_decision = g_politicas->data[p][i];
            out->data[p][i]= x->data[i];
            out->data[p][NUM_ESTADOS] += x->data[i] * g_costos->data[j++][politica_decision - 1];
        }
        free_matrix(mat_politica);
        free_matrix(A); free_vector(b); free_vector(x);
    }
    return out; 
}

int return_optimal(Matrix *EEP_Solution){
    int index=0; double costo = DBL_MAX;
    bool maximizar = false;
    if (strcmp(TIPO, "max") == 0) { costo *= -1; maximizar = true;}

    FOREACH_INDEX(i, EEP_Solution->rows){
        double costo_politica = EEP_Solution->data[i][NUM_ESTADOS];
        if ((maximizar && costo_politica>costo) || (!maximizar && costo_politica<costo)) {
            index = i;
            costo = EEP_Solution->data[i][NUM_ESTADOS];
        }
    }

    return index+1;
}