#include <stdio.h>
#include "ficheros.h"

int main(int argc, char **argv){
     
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        fprintf(stderr, RED "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    mi_chmod_f(atoi(argv[2]), atoi(argv[3]));

    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}