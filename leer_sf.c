#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "ficheros_basico.h"

#define NIVEL2 1
#define NIVEL3 1

struct superbloque SB;

int leer_superbloque(){
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %u\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %u\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %u\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %u\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %u\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %u\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %u\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %u\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %u\n" , SB.cantBloquesLibres );
    printf("cantInodosLibres = %u\n", SB.cantInodosLibres);
    printf("totBloques = %u\n", SB.totBloques);
    printf("totInodos = %u\n", SB.totInodos);

    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    return EXITO;
}

int print_inodos(){
    unsigned int inodosPorBloque = BLOCKSIZE / INODOSIZE;
    struct inodo inodos[inodosPorBloque];
    int posInodo;
    
    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");

    for (int nbloque = SB.posPrimerBloqueAI; nbloque < SB.posUltimoBloqueAI ; nbloque++){
        // Leer bloque completo de inodos
        if (bread(nbloque, inodos) == FALLO) {
            bumount();
            return FALLO;
        }

        // Printearlos
        for(int j = 0; j < inodosPorBloque; j++){
            posInodo = inodos[j].punterosDirectos[0];
            if(posInodo != UINT_MAX){
                printf("%u, ",inodos[j].punterosDirectos[0]);
            } else {
                break;
            }
        }
    }

    printf("-1\n");
    return EXITO;
}

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "Sintaxis: %s <nombre_dispositivo>\n", argv[0]);
        return FALLO;
    }

    // Montar dispositivo
    if(bmount(argv[1]) == FALLO) return FALLO;

    // Leer superbloque (bloque 0)
    if(bread(posSB, &SB) == FALLO){
        bumount();
        return FALLO;
    }

    #if NIVEL2 || NIVEL3
        leer_superbloque();
    #endif

    // Print y lectura de los inodos

    #if NIVEL2
        print_inodos();
    #endif


    bumount();
    return EXITO;
}