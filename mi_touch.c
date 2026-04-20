#include "directorios.h"

int main(int argc, char **argv){
    
    //Comprobamos la sintaxis
    if(argc != 3){
        fprintf(stderr, RED "Sintaxis: ./mi_touch <nombre_dispositivo> <permisos> </ruta_directorio/>" RESET);
        return FALLO;
    }

    //Comprobamos si es un fichero
    if(argv[3][strlen(argv[3]) - 1] == '/'){
        fprintf(stderr, RED "Error, (poner mensaje adecuado)" RESET);
        return FALLO;
    }

    //Comprobamos los permisos
    if(atoi(argv[2]) < 0 || atoi(argv[2]) < 0){
        fprintf(stderr, RED "Error, (poner mensaje adecuado)" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    if(mi_creat(argv[3], atoi(argv[2])) == FALLO){
        return FALLO;
    }

    bumount();
}