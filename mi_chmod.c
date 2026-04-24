#include "directorios.h"

int main(int argc, char **argv){

    //Comprobamos la sintaxis
    if(argc != 4){
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n" RESET);
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

    // Llamar a mi_chmod con la ruta (argv[3]) y los permisos
    if(mi_chmod(argv[3], permisos) == FALLO){
        bumount();
        return FALLO;
    }

    bumount();
    return EXITO;
}