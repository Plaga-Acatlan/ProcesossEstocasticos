#include "setup.h"
#include "utils.h"
#include "datatype.h"
#include <string.h>

#ifndef Simplex_H
#define Simplex_H

char *FO();
char *CondicionNormalizacion();
int Make_Restiction(int state_idx, char *buffer, int buffer_size);
char *Make_Restrictions();
char *Create_MPL();

#endif