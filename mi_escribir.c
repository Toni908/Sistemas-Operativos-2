#include "directorios.h"

int main(int argc, char **argv){
    
    // Comprobamos la sintaxis
    if(argc != 4){
        fprintf(stderr, RED "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }
 
    //Comprobamos si es un fichero
    if ((argv[2][strlen(argv[2]) - 1]) == '/'){
        fprintf(stderr, "No es un fichero\n");
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // disco virtual
        return FALLO;
    }

    int bytesEscritos = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));

    bumount();
    return EXITO;
}