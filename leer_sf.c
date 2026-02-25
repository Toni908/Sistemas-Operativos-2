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

    struct inodo inodoSize;
    printf("Tamaño de Inodo (Bytes): %lu", sizeof(inodoSize));

    // Print y lectura de los inodos

    unsigned int inodosPorBloque = BLOCKSIZE / INODOSIZE;
    struct inodo inodos[inodosPorBloque];
    int posInodo = 0;

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");

    while (posInodo != UINT_MAX) {

        // Calcular bloque del AI donde está el inodo
        unsigned int nbloque = SB.posPrimerBloqueAI + (posInodo / inodosPorBloque);

        // Leer bloque completo de inodos
        if (bread(nbloque, inodos) == FALLO) {
            perror("Error al leer bloque del AI");
            bumount();
            return FALLO;
        }

        // Printearlos
        for(int j = 0; j <= inodosPorBloque; j++){
            if(posInodo != UINT_MAX){
                printf("%u, ",inodos[j].punterosDirectos[0]);
                posInodo = inodos[j].punterosDirectos[0];
            }
        }
    }

    printf("-1\n");

    bumount();
    return EXITO;
}