#include <stdlib.h>
#include <stdio.h>
#include "tool.h"
#include <string.h>

// Variables que llegan y se deben cargar
const int NUM_ESTADOS = 2;
const int NUM_POLITICAS = 4;
const int NUM_DECISIONES = 2;
const char TIPO[3] = "Min";

// Matrices de trancisión
const Matrix K[NUM_ESTADOS][NUM_ESTADOS][NUM_DECISIONES];
const Matrix()