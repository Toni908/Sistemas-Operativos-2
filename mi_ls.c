#include "directorios.h"

#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) //suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

int main(int argc, char **argv){
    
    //Comprobamos la sintaxis
    if(argc != 3){
        fprintf(stderr, RED "Poner el error sintaxis" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    //...

    bumount();
}