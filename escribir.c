//Antonio García Font y Maria Isabel Herrero Soteras
#include <stdio.h>
#include "ficheros.h"

int main(int argc, char **argv){
    int  offsets [] = {9000, 209000, 30725000,409605000,480000000};
    int num_offsets = sizeof(offsets) / sizeof(offsets[0]);
    struct  STAT stat;
    
    //Comprobamos la sintaxis
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        fprintf(stderr, RED "Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        fprintf(stderr,"Offsets: ");
        for(int i = 0; i < num_offsets; i++){
            fprintf(stderr,"%d ", offsets[i]);
        }
        fprintf(stderr,"\n");
        fprintf(stderr,"Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    printf("Longitud del texto %ld\n", strlen(argv[2]));
    printf("\n");

    if(atoi(argv[3]) == 0){ //Caso de que se reserva un inodo para todos los offsets
        unsigned int ninodo = reservar_inodo('f',6);
        for(int i = 0; i < num_offsets; i++){
            printf("Nº de inodo reservado: 1\n");
            printf("Offset: %d\n", offsets[i]);
            mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));
            mi_stat_f(ninodo, &stat);
            printf("Bytes escritos: %ld\n", strlen(argv[2]));
            printf("stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados = %d\n", stat.numBloquesOcupados);
            printf("\n");
        }
    }else{ //Caso de que se reserva un inodo diferente para cada offset
        for(int i = 0; i < num_offsets; i++){
            unsigned int ninodo = reservar_inodo('f',6);
            printf("Nº de inodo reservado: %d\n", ninodo);
            printf("Offset: %d\n", offsets[i]);
            mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));
            mi_stat_f(ninodo, &stat);
            printf("Bytes escritos: %ld\n", strlen(argv[2]));
            printf("stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados = %d\n", stat.numBloquesOcupados);
            printf("\n");
        }
    }
    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}