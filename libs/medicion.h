#ifndef medicion_h
#define medicion_h

#include "lectura.h"

struct medicion {
    char idpac[50];
    char fecha[50];
    int cantlect;
    lectura* datos;
};

#endif
