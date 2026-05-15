#include "Simplex.h"

typedef struct {
    int EDO;  
    int DEC;
    double COEF; 
} Term;

char *FO(){
    char *fo = (char*)malloc(sizeof(char));
    if (!fo) {
        log_error("FO", "Fallo de memoria al asignar", -1);
        return NULL;
    }

    bool counter = false;

    for(int i=0; i<g_costos->cols; i++){
        for(int j=0; j<g_costos->rows; j++){
            if (g_costos->data[j][i] != DBL_MAX) {
                char buffer[50];
                if (g_costos->data[j][i] > 0) 
                    (counter) ? snprintf(buffer, sizeof(buffer), " + %.2fy_{%d,%d}", g_costos->data[j][i], j, i+1) : 
                    snprintf(buffer, sizeof(buffer), "%.2fy_{%d,%d}", g_costos->data[j][i], j, i+1);
                else if(g_costos->data[j][i] < 0) 
                    snprintf(buffer, sizeof(buffer), " - %.2fy_{%d,%d}", -g_costos->data[j][i], j, i+1);
                
                int new_length = strlen(fo) + strlen(buffer) + 1;
                char *new_fo = (char*)realloc(fo, new_length);
                if (!new_fo) {
                    log_error("FO", "Fallo de memoria al expandir", -1);
                    free(fo);
                    return NULL;
                 }
                fo = new_fo;
                strcat(fo, buffer);
                if (g_costos->data[j][i] != 0) counter = true;
            }
        }
    }
    return fo;
}

char *CondicionNormalizacion(){
    char *condicion = (char*)malloc(sizeof(char));
    if (!condicion) {
        log_error("CondicionNormalizacion", "Fallo de memoria al asignar", -1);
        return NULL;
    }

    bool counter = false;

    for(int i=0; i<NUM_ESTADOS; i++){
        for(int k=0; k<g_transiciones_3d->depth; k++){
            char buffer[50];
            if (g_costos->data[i][k] == DBL_MAX) continue;
            (counter) ? snprintf(buffer, sizeof(buffer), " + y_{%d,%d}", i, k+1) : 
            snprintf(buffer, sizeof(buffer), "y_{%d,%d}", i, k+1);

            int new_length = strlen(condicion) + strlen(buffer) + 1;
            char *new_condicion = (char*)realloc(condicion, new_length);
            if (!new_condicion) {
                log_error("CondicionNormalizacion", "Fallo de memoria al expandir", -1);
                free(condicion);
                return NULL;
                }
            condicion = new_condicion;
            strcat(condicion, buffer);
            counter = true;
        }
    }
    strcat(condicion, " = 1");
    return condicion;
}

static int accumulate_term(Term *terms, int *term_count, int max_terms, 
                          int state, int decision, double coeff) {
    // Buscar si ya existe este término
    for (int t = 0; t < *term_count; t++) {
        if (terms[t].EDO == state && terms[t].DEC == decision) {
            terms[t].COEF += coeff;
            return 0;
        }
    }
    
    // Agregar nuevo término
    if (*term_count >= max_terms) return -1; 
    
    terms[*term_count].EDO = state;
    terms[*term_count].DEC = decision;
    terms[*term_count].COEF = coeff;
    (*term_count)++;
    
    return 0;
}

int Make_Restiction(int state_idx, char *buffer, int buffer_size) {
    if (state_idx < 0 || state_idx >= NUM_ESTADOS-1) return -1;
    
    char coeff_buf[32];
    
    int max_terms = NUM_ESTADOS * NUM_DECISIONES;
    Term *terms = (Term*)calloc(max_terms, sizeof(Term));
    if (!terms) return -1;
    
    int term_count = 0;
    
    // 1. Positivos
    for (int k = 0; k < NUM_DECISIONES; k++) {
        if(g_costos->data[state_idx][k] == DBL_MAX) continue;
        accumulate_term(terms, &term_count, max_terms, state_idx, k, +1.0);
    }
    
    // 2. Negativos
    for (int k = 0; k < NUM_DECISIONES; k++) { 
        for (int j = 0; j < NUM_ESTADOS; j++) { 
            double prob = g_transiciones_3d->data[k][j][state_idx];
            
            if (prob < 0) continue; 
            
            // Agregar término: -P_{state,j}(k) * y_{j,k}
            accumulate_term(terms, &term_count, max_terms, j, k, -prob);
        }
    }
    
    // 3. String
    int pos = 0;
    bool first_term = true;

    for (int t = 0; t < term_count; ++t) {
        const Term *term = &terms[t];
        double coeff = term->COEF;
        
        if (is_approx_zero(coeff)) continue;
        
        // Signo y separador 
        if (!first_term) {
            int written = snprintf(buffer + pos, buffer_size - pos, " %c ", coeff > 0 ? '+' : '-');
            if (written > 0 && pos + written < buffer_size) pos += written;
        } else if (coeff < 0) {
            int written = snprintf(buffer + pos, buffer_size - pos, "-");
            if (written > 0 && pos + written < buffer_size) pos += written;
        }
        
        // Coeficiente
        double abs_coeff = abs_double(coeff);
        if (!is_approx_zero(abs_coeff) && !is_approx_zero(abs_coeff - 1.0)) {
            snprintf(coeff_buf, sizeof(coeff_buf), "%.4f*", abs_double(coeff));
            int written = snprintf(buffer + pos, buffer_size - pos, "%s", coeff_buf);
            if (written > 0 && pos + written < buffer_size) pos += written;
        }
        
        // y_{estado,decisión}
        int written = snprintf(buffer + pos, buffer_size - pos, "y_{%d,%d}", 
                              term->EDO, term->DEC + 1);
        if (written > 0 && pos + written < buffer_size) pos += written;
        
        first_term = false;
    }
    pos += snprintf(buffer + pos, buffer_size - pos, " = 0");
    buffer[buffer_size - 1] = '\0';
    
    free(terms);
    
    return 0;
}

char *Make_Restrictions() { 
    int capacity = 2048;
    char *result = malloc(capacity);
    if (!result) return NULL;

    result[0] = '\0';
    int actual = 0;

    for (int s = 0; s < NUM_ESTADOS-1; s++) {
        char temp_buf[2048];
        if (Make_Restiction(s, temp_buf, sizeof(temp_buf)) != 0) {
            continue;  // Saltar estados con error
        }
        int append_len = strlen(temp_buf);
        int needed = actual + append_len + 2;
        if (needed >= capacity) {
            capacity = needed * 2;
            char *new_result = realloc(result, capacity);
            if (!new_result) {
                free(result);
                return NULL; 
            }
            result = new_result;
        }
        actual += snprintf(result + actual, capacity - actual, 
                          "%s\n", temp_buf);
    }
    
    return result;
}

char *Create_MPL(){
    char *fo = FO();
    char *condicion = CondicionNormalizacion();
    char *restricciones = Make_Restrictions();
    
    int total_length = strlen(fo) + strlen(condicion) + strlen(restricciones) + 22;
    char *full_output = (char*)malloc(total_length);
    if (!full_output) {
        log_error("Call", "Fallo de memoria al asignar", -1);
        free(fo);
        free(condicion);
        free(restricciones);
        return NULL;
    }
    
    snprintf(full_output, total_length, "%s z = %s\n%s\n%s\ny_{i,k}>=0", ((maximizar) ? "Max" : "Min"), fo, condicion, restricciones);
    
    free(fo);
    free(condicion);
    free(restricciones);
    
    return full_output;
}