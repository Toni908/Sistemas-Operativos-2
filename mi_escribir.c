#include "directorios.h"

int main(int argc, char **argv){
    
    // Comprobamos la sintaxis
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL){
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
    printf("Longitud del texto: %ld\n", strlen(argv[3]));
    printf("Bytes escritos %d\n", bytesEscritos);
    bumount();
    return EXITO;
}