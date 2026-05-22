//Antonio García Font y Maria Isabel Herrero Soteras  
#include "directorios.h"

int main(int argc, char **argv){

    // comprobar sintaxis
    if (argc != 3){
        fprintf(stderr, "Sintaxis: ./mi_rm disco /ruta\n");
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

    // Si es un directorio, denegamos el borrado
    if (stat.tipo == 'd') {
        fprintf(stderr, "Error: La ruta '%s' es un directorio. Por favor, usa mi_rmdir.\n", argv[2]);
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