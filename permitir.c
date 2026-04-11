//Antonio García Font y Maria Isabel Herrero Soteras
#include <stdio.h>
#include "ficheros.h"

int main(int argc, char **argv){
     
    //Comprobamos la sintaxis
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        fprintf(stderr, RED "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    mi_chmod_f(atoi(argv[2]), atoi(argv[3])); //Cambiamos ls permisos

    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}