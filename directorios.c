#include "directorios.h"

#define NIVEL7 1

#define DEBUG 1

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    // Comprobar que el camino empieza por '/'
    if (camino[0] != '/') {
        return -1; // ERROR
    }

    // Si el camino es solo "/"
    if (strcmp(camino, "/") == 0) {
        strcpy(inicial, "/");
        strcpy(final, "");
        *tipo = 'd';
        return 0;
    }

    // Buscar el siguiente '/'
    const char *p = strchr(camino + 1, '/');

    if (p == NULL) {
        // No hay más '/', es el último componente (fichero)
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        *tipo = 'f';
    } else {
        // Hay más niveles (directorio)
        int len = p - (camino + 1);

        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0';

        strcpy(final, p);
        *tipo = 'd';
    }

    return 0;
}