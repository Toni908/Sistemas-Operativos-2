#include "directorios.h"

int main(int argc, char **argv){

    //Comprobamos la sintaxis
    if(argc != 3){
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    //...

    bumount();
}