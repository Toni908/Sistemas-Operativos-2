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

    struct STAT stat;
    if (mi_stat(argv[2], &stat) == FALLO) {
        bumount();
        return FALLO;
    }

  
    if (stat.tipo != 'd') {
        fprintf(stderr, "Error: La ruta '%s' no es un directorio. Por favor, usa mi_rm para borrar ficheros.\n", argv[2]);
        bumount();
        return FALLO;
    }

    if (mi_unlink(argv[2]) == FALLO){
        bumount();
        return FALLO;
    }

    bumount();

    return EXITO;
}