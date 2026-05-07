#include "directorios.h"

int main(int argc, char **argv){

    //Comprobamos la sintaxis
    if(argv[0] == NULL || argv[1] == NULL){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    //Comprobamos el directorio raiz
    char *aux = argv[2];
    if (strcmp(aux, "/") == 0) {
        fprintf(stderr, "Es el directorio raiz\n");
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // disco virtual
        return FALLO;
    }

    //mi_unlink(argv[2], false);
    bumount();
}