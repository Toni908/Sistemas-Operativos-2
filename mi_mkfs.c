//Antonio García Font y Maria Isabel Herrero Soteras  
#include <stdio.h>
#include "bloques.h"
#include "ficheros_basico.h"

#define NIVEL2 0
#define NIVEL3 0
#define NIVEL4 0
#define NIVEL5 1

int main(int argc, char **argv){
    unsigned char buffer[BLOCKSIZE];
    unsigned int nbloques = atoi(argv[2]);

    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    memset(buffer, 0 , BLOCKSIZE); 
    for(int i = 0; i < nbloques; i++){ //bucle for para escribir los bloques
        bwrite(i, buffer);
    }

    unsigned int ninodos = nbloques / 4;   // Heuristica

    initSB(nbloques, ninodos);
    initMB();
    initAI();

    #if NIVEL3 || NIVEL4 || NIVEL5
        reservar_inodo('d', 7);
    #endif

    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}
