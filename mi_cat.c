#include "directorios.h"

#define TAMBUFFER 1500

int main(int argc, char **argv){
    
    // Comprobamos la sintaxis 
    if(argv[1] == NULL || argv[2] == NULL){
        fprintf(stderr, RED "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }

    //Comprobamos si es un fichero (NO debe terminar en '/')
    if(argv[2][strlen(argv[2]) - 1] == '/'){
        fprintf(stderr, RED "Error: la ruta no debe terminar en '/' para crear un fichero\n" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // disco virtual
        return FALLO;
    }

    char buffer[TAMBUFFER];
    int leidos = 0;
    unsigned int offset = 0;

    // Leer bloque a bloque
    memset(buffer, 0, TAMBUFFER);
    leidos = mi_read(argv[2], buffer, offset, TAMBUFFER);

    bumount();
    return EXITO;
}