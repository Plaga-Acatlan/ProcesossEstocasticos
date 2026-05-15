#ifndef UTILS_H
#define UTILS_H

#include "datatype.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #define isatty_win _isatty
    #define fileno_win _fileno
#else
    #include <unistd.h>
    #define isatty_win isatty
    #define fileno_win fileno
#endif

// === Constantes generales ===
#ifndef EPSILON
#define EPSILON 1e-9
#endif

#ifndef MAX_LOG_LENGTH
#define MAX_LOG_LENGTH 512
#endif

// Colores ANSI para terminal (si se soportan)
#define COLOR_RESET     "\033[0m"
#define COLOR_BLUE      "\033[1;34m"  
#define COLOR_ORANGE    "\033[38;5;208m"
#define COLOR_RED       "\033[1;31m" 
#define COLOR_YELLOW    "\033[1;33m"

// Verifica que un puntero no sea NULL y termina el programa con mensaje si lo es.
void assert_not_null(void* ptr, const char* msg, const char* file, int line);

// Macro de conveniencia para assert_not_null
#define ASSERT_NOT_NULL(ptr, msg) assert_not_null(ptr, msg, __FILE__, __LINE__)


// === Utilidades matemáticas ===
// Retorna el valor absoluto de un double.
double abs_double(double value);

// Compara dos valores double con tolerancia EPSILON.
bool double_equals(double a, double b);

// Verifica si un valor es aproximadamente cero.
bool is_approx_zero(double value);

// Limita un valor dentro de un rango [min, max].
double clamp_value(double value, double min, double max);

// Retorna el signo de un valor: -1 para negativo, 0 para cero, 1 para positivo.
int sign_of(double value);


// === Utilidades de entrada/validación ===
void clear_input_buffer(void);


// === Logging y debugging ===

// Imprime un mensaje de log con formato y nivel (INFO, ERROR, etc.).
void log_message(const char* level, const char* format, ...);

// Imprime un mensaje de error con información contextual (función, mensaje, código).
void log_error(const char* function, const char* message, int error_code);

// Imprime separador visual para debugging en consola.
void print_separator(char char_char, int width);


// === Utilidades de tiempo ===

// Retorna timestamp actual en formato legible "YYYY-MM-DD HH:MM:SS".
char* get_timestamp(char* buffer, size_t buffer_size);

// Mide tiempo de ejecución de un bloque usando clock().
void start_execution_timer(clock_t* start_time);
double end_execution_timer(const clock_t* start_time);


// === Utilidades para arrays/colecciones ===

// Intercambia dos valores double por referencia.
void swap_doubles(double* a, double* b);

// Intercambia dos punteros genéricos por referencia.
void swap_pointers(void** a, void** b);

// Verifica si un array de ints contiene un valor específico.
bool array_contains_int(const int* arr, int size, int value);

// Encuentra el índice de un valor en un array de ints, o -1 si no existe.
int find_index_in_array(const int* arr, int size, int value);

// === Otras utilidades generales ===
// Función para resolver un sistema de ecuaciones lineales usando Gauss-Jordan
// A es la matriz de coeficientes, b es el vector de términos independientes
int gauss_jordan(Matrix *A, Vector *b, Vector *x);

Vector* multiply_matrix_vector(const Matrix *A, const Vector *x);

bool is_vector_equals(const Vector *a, const Vector *b);

const char* get_log_color(const char* level);
bool terminal_supports_color(FILE* stream);

const char* format_coefficient(double abs_coeff, char *temp_buf, size_t buf_size);

// === Macros de conveniencia ===
#define FOREACH_INDEX(i, n) for (size_t i = 0; i < (size_t)(n); ++i)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define UNUSED(x) (void)(x)
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define sign_char(x) ((x) < 0 ? '-' : '+')

// Para branch prediction hints 
#ifndef likely
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif // UTILS_H