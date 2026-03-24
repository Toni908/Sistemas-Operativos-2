#include <stdio.h>
#include "ficheros_basico.h"

int main(int argc, char **argv){
     
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        fprintf(stderr, RED "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    if(atoi(argv[3]) == 0){ //si nbytes es igual a 0
        liberar_inodo(atoi(argv[2]));
    }else{
        mi_truncar_f(atoi(argv[2]),atoi(argv[3]));
    }

    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}