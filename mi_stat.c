#include "directorios.h"

int main(int argc, char **argv){
    //Comprobamos la sintaxis
    if(argc != 2){
        fprintf(stderr, RED "Poner mensaje adecuado de error" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    //...

    bumount();
}