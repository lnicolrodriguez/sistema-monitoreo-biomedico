#ifndef configuracion_h
#define configuracion_h

struct umbralsensor {
    char tipo[50];
    double vmin;
    double vmax;
};

struct configuracion {
    umbralsensor* lista;
    int cant;
};

#endif
