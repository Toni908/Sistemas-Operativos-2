#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "ficheros_basico.h"

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "Sintaxis: %s <nombre_dispositivo>\n", argv[0]);
        return FALLO;
    }

    // Montar dispositivo
    if(bmount(argv[1]) == FALLO){
        perror(RED "Error"); 
        return FALLO;
    }

    struct superbloque SB;

    // Leer superbloque (bloque 0)
    if(bread(posSB, &SB) == FALLO){
        perror(RED "Error"); 
        bumount();
        return FALLO;
    }

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %u\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %u\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %u\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %u\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %u\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %u\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %u\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %u\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %u\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %u\n", SB.cantInodosLibres);
    printf("totBloques = %u\n", SB.totBloques);
    printf("totInodos = %u\n", SB.totInodos);

    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");

    unsigned int posInodo = SB.posPrimerInodoLibre;
    struct inodo inodo;

    while(posInodo != UINT_MAX){

        // Leer inodo manualmente (por si no tienes leer_inodo implementado)
        unsigned int inodosPorBloque = BLOCKSIZE / INODOSIZE;
        unsigned int bloqueAI = SB.posPrimerBloqueAI + (posInodo / inodosPorBloque);
        unsigned int desplazamiento = posInodo % inodosPorBloque;

        struct inodo buffer[inodosPorBloque];

        if(bread(bloqueAI, buffer) == FALLO){
            perror(RED "Error"); 
            bumount();
            return FALLO;
        }

        inodo = buffer[desplazamiento];

        printf("%u ", posInodo);

        posInodo = inodo.punterosDirectos[0];
    }

    printf("-1\n");

    bumount();
    return EXITO;
}