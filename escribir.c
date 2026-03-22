#include <stdio.h>
#include "ficheros_basico.h"

int main(int argc, char **argv){
    int  offsets [] = {9000, 209000, 30725000,409605000,480000000};
    struct  STAT *stat;
    
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        fprintf(stderr, RED "ERROR" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    
    if(argv[3] == 0){ //caso de que se reserva un inodo para todos los offsets
        unsigned int ninodo = reservar_inodo('f',6);
        for(int i = 0; offsets[i] != NULL; i++){
            mi_write_f(ninodo, argv[2], offsets[i], sizeof(argv[2]));
            mi_stat_f(ninodo, stat);
        }
    }else{
        for(int i = 0; offsets[i] != NULL; i++){
            unsigned int ninodo = reservar_inodo('f',6);
            mi_write_f(ninodo, argv[2], offsets[i], sizeof(argv[2]));
            mi_stat_f(ninodo, stat);
        }
    }
    //falta hacer la prueba del read y hacer los prints y testearlo en general
    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}