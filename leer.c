#include <stdio.h>
#include "ficheros_basico.h"

#define tambuffer 1500

int main(int argc, char **argv){
    int offset = 0;
    int leidos = 0;
    unsigned char buffer[tambuffer];

    if(argv[1] == NULL || argv[2] == NULL){
        fprintf(stderr, RED "ERROR" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    leidos = mi_read_f(atoi(argv[2]), buffer, offset, tambuffer);
    printf("%s\n", buffer);
    while(leidos > 0){
        leidos = mi_read_f(atoi(argv[2]), buffer, offset, tambuffer);
        printf("%s\n", buffer);
    }
    printf("total_leidos");
    printf(CYAN "%d\n" RESET, leidos);
    printf("tamEnBytesLog");
    printf(CYAN "%d\n" RESET, leidos); //TAM EN BYTES LOG SIEMPRE COINCIDE CON LEIDOS ????
    bumount();  //desmontamos el dispostivo virtual
    return EXITO;
}