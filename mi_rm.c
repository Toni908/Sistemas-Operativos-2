#include "directorios.h"

int main(int argc, char **argv){

    // comprobar sintaxis
    if (argc != 3){
        fprintf(stderr, "Sintaxis: ./mi_rm disco /ruta\n");
        return FALLO;
    }

    // no permitir borrar raiz
    if (strcmp(argv[2], "/") == 0){
        fprintf(stderr, "Error: No se puede borrar el directorio raíz\n");
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO){
        return FALLO;
    }

    if (mi_unlink(argv[2]) == FALLO){
        bumount();
        return FALLO;
    }

    bumount();

    return EXITO;
}