#include "utils.h"
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <float.h>

void assert_not_null(void* ptr, const char* msg, const char* file, int line) {
    if (!ptr) {
        fprintf(stderr, "FATAL: %s en %s:%d\n", msg, file, line);
        fprintf(stderr, "Terminando ejecución por puntero NULL inesperado.\n");
        exit(EXIT_FAILURE);
    }
}


// UTILIDADES MATEMÁTICAS
double abs_double(double value) {
    return value < 0 ? -value : value;
}

bool double_equals(double a, double b) {
    return abs_double(a - b) < EPSILON;
}

bool is_approx_zero(double value) {
    return abs_double(value) < EPSILON;
}

double clamp_value(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int sign_of(double value) {
    if (value > EPSILON) return 1;
    if (value < -EPSILON) return -1;
    return 0;
}

// UTILIDADES DE ENTRADA/VALIDACIÓN
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


// LOGGING Y DEBUGGING
void log_message(const char* level, const char* format, ...) {
    if (!level || !format) return;
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    va_list args;
    va_start(args, format);
    
    char message[MAX_LOG_LENGTH];
    vsnprintf(message, sizeof(message), format, args);
    
    va_end(args);
    
    if (terminal_supports_color(stdout)) {
        const char* color = get_log_color(level);
        fprintf(stdout, "%s[%s] [%s] %s%s\n", 
                color, timestamp, level, message, COLOR_RESET);
    } else {
        // Sin colores: formato plano
        fprintf(stdout, "[%s] [%s] %s\n", timestamp, level, message);
    }
    fflush(stdout);
}

void log_error(const char* function, const char* message, int error_code) {
    if (!function || !message) return;
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    if (error_code != 0) {
        fprintf(stderr, "[%s] [ERROR] %s(): %s (código: %d)\n", 
                timestamp, function, message, error_code);
    } else {
        fprintf(stderr, "[%s] [ERROR] %s(): %s\n", 
                timestamp, function, message);
    }
    fflush(stderr);
}

void print_separator(char char_char, int width) {
    if (width <= 0) return;
    for (int i = 0; i < width; i++) {
        putchar(char_char);
    }
    putchar('\n');
}

// UTILIDADES DE TIEMPO

char* get_timestamp(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 20) return NULL;
    
    time_t now = time(NULL);
    struct tm* time_info = localtime(&now);
    
    if (!time_info) {
        buffer[0] = '\0';
        return NULL;
    }
    
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", time_info);
    return buffer;
}

void start_execution_timer(clock_t* start_time) {
    if (start_time) {
        *start_time = clock();
    }
}

double end_execution_timer(const clock_t* start_time) {
    if (!start_time) return -1.0;
    
    clock_t end = clock();
    return (double)(end - *start_time) / CLOCKS_PER_SEC;
}

// UTILIDADES PARA ARRAYS/COLECCIONES

void swap_doubles(double* a, double* b) {
    if (!a || !b) return;
    double temp = *a;
    *a = *b;
    *b = temp;
}

void swap_pointers(void** a, void** b) {
    if (!a || !b) return;
    void* temp = *a;
    *a = *b;
    *b = temp;
}

bool array_contains_int(const int* arr, int size, int value) {
    if (!arr || size <= 0) return false;
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) return true;
    }
    return false;
}

int find_index_in_array(const int* arr, int size, int value) {
    if (!arr || size <= 0) return -1;
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) return i;
    }
    return -1;
}

int gauss_jordan(Matrix *A, Vector *b, Vector *x) {
    if (!A || !b || !x || A->rows != A->cols || A->rows != b->size || A->rows != x->size) {
        log_error("gauss_jordan", "Dimensiones incompatibles o punteros NULL", -1);
        return -1;
    }
    int n = A->rows;

    // Crear matriz aumentada
    Matrix *aug = create_matrix(n, n + 1);
    if (!aug) return -1;

    // Copiar A y b a la matriz aumentada
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            aug->data[i][j] = A->data[i][j];
        }
        aug->data[i][n] = b->data[i];
    }

    // Gauss-Jordan
    for (int p = 0; p < n; p++) {
        // Encontrar pivote
        int max = p;
        for (int i = p + 1; i < n; i++) {
            if (abs_double(aug->data[i][p]) > abs_double(aug->data[max][p])) {
                max = i;
            }
        }
        // Intercambiar filas
        if (p != max) {
            double *temp = aug->data[p];
            aug->data[p] = aug->data[max];
            aug->data[max] = temp;
        }

        // Verificar si pivote es cero
        if (is_approx_zero(aug->data[p][p])) {
            free_matrix(aug);
            log_error("gauss_jordan", "Matriz singular", -1);
            return -1;
        }

        // Hacer pivote 1
        double pivot = aug->data[p][p];
        for (int j = 0; j <= n; j++) {
            aug->data[p][j] /= pivot;
        }

        // Eliminar otras filas
        for (int i = 0; i < n; i++) {
            if (i != p) {
                double factor = aug->data[i][p];
                for (int j = 0; j <= n; j++) {
                    aug->data[i][j] -= factor * aug->data[p][j];
                }
            }
        }
    }

    // Extraer solución
    for (int i = 0; i < n; i++) {
        x->data[i] = aug->data[i][n];
    }

    free_matrix(aug);
    return 0;
}

bool terminal_supports_color(FILE* stream) {
    if (!stream) return false;
    if (getenv("NO_COLOR") != NULL) return false;
    
    
    #ifdef _WIN32
        return _isatty(_fileno(stream));
    #else
        return isatty(fileno(stream));
    #endif
}

const char* get_log_color(const char* level) {
    if (!level) return COLOR_RESET;
    
    if (strcmp(level, "ERROR") == 0) {
        return COLOR_RED;
    } else if (strcmp(level, "DEBUG") == 0) {
        if (terminal_supports_color(stdout)) {
            return COLOR_ORANGE;
        }
        return COLOR_YELLOW;
    } else if (strcmp(level, "INFO") == 0) {
        return COLOR_BLUE;
    } else if (strcmp(level, "WARN") == 0) {
        return COLOR_YELLOW;
    }
    
    return COLOR_RESET;  // Por defecto, sin color
}

Vector* multiply_matrix_vector(const Matrix *A, const Vector *x) {
    if (A == NULL || x == NULL) {
        log_error("multiply_matrix_vector", "Punteros NULL recibidos", -1);
        return NULL;
    }
    if (A->cols != x->size) {
        log_error("multiply_matrix_vector", "Dimensiones incompatibles", -1);
        return NULL;
    }

    Vector *y = create_vector(A->rows);
    if (y == NULL) return NULL;

    for (int i = 0; i < A->rows; i++) {
        double sum = 0.0;
        for (int j = 0; j < A->cols; j++) {
            sum += A->data[i][j] * x->data[j];
        }
        y->data[i] = sum;
    }

    return y;
}

bool is_vector_equals(const Vector *a, const Vector *b) {
    if (!a || !b || a->size != b->size || !a->data || !b->data) return false;
    for (int i = 0; i < a->size; i++) {
        if (abs_double(a->data[i] - b->data[i]) > 1e-9) return false;
    }
    return true;
}