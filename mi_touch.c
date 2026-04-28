#include "directorios.h"

int main(int argc, char **argv){
    
    // Comprobamos la sintaxis (./mi_touch disco permisos /ruta)
    if(argc != 4){
        fprintf(stderr, RED "Sintaxis: ./mi_touch <nombre_dispositivo> <permisos> </ruta_fichero>\n" RESET);
        return FALLO;
    }

    //Comprobamos si es un fichero (NO debe terminar en '/')
    if(argv[3][strlen(argv[3]) - 1] == '/'){
        fprintf(stderr, RED "Error: la ruta no debe terminar en '/' para crear un fichero\n" RESET);
        return FALLO;
    }

    //Comprobamos los permisos (rango 0-7)
    int permisos = atoi(argv[2]);
    if(permisos < 0 || permisos > 7){
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        return FALLO;
    }
 
    if(bmount(argv[1]) == FALLO){ // disco virtual
        return FALLO;
    }

    int error = mi_creat(argv[3], permisos);
    if(error <= FALLO){
        mostrar_error_buscar_entrada(error);
        bumount();
        return FALLO;
    }

    bumount();
    return EXITO;
}