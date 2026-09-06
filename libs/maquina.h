#ifndef maquina_h
#define maquina_h

#include "medicion.h"

struct maquinauci {
    char idmaq;
    int cantmed;
    medicion* lista;
};

#endif
