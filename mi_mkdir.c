#include "directorios.h"

int main(int argc, char **argv){
    
    //Comprobamos la sintaxis (maribel si tiene 4 entonces mal, no pruebes con el != 3)
    if(argc != 4){  
        fprintf(stderr, RED "Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n" RESET);
        return FALLO;
    }

    //Comprobamos si es un directorio (debe terminar en /)
    if(argv[3][strlen(argv[3]) - 1] != '/'){
        fprintf(stderr, RED "Error: la ruta debe terminar en '/' para crear un directorio\n" RESET);
        return FALLO;
    }

    //Comprobamos los permisos (rango 0-7)
    int permisos = atoi(argv[2]);
    if(permisos < 0 || permisos > 7){ 
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){
        return FALLO;
    }

    int error = mi_creat(argv[3], permisos); 
    fprintf("%d", error);
    if(error <= FALLO){
        // Mostrar el error específico
        mostrar_error_buscar_entrada(error);
        bumount();
        return FALLO;
    }
    bumount();
    return EXITO;
}