#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "ficheros_basico.h"


#define NIVEL2 0
#define NIVEL3 0
#define NIVEL4 0
#define NIVEL5 1

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

    #if NIVEL2
        printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
        printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));
    #endif

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

int mostrar_mapa_bits() {
    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");

    // leer bit de superbloque
    printf("posSB: %d → leer_bit(%d) = %d\n", posSB, posSB, leer_bit(posSB));
    // bit comienzo MB
    printf("SB.posPrimerBloqueMB: %u → leer_bit(%u) = %d\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    // bit final MB
    printf("SB.posUltimoBloqueMB: %u → leer_bit(%u) = %d\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    // bit comienzo AI
    printf("SB.posPrimerBloqueAI: %u → leer_bit(%u) = %d\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    // bit final AI
    printf("SB.posUltimoBloqueAI: %u → leer_bit(%u) = %d\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    // bit comienzo Datos
    printf("SB.posPrimerBloqueDatos: %u → leer_bit(%u) = %d\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    // bit final Datos
    printf("SB.posUltimoBloqueDatos: %u → leer_bit(%u) = %d\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

    return EXITO;
}

int prueba_reservar_liberar_bloque() {
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int nbloque = reservar_bloque();
    if (nbloque == FALLO) return FALLO;
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", nbloque);

    // Leer SB actualizado
    if(bread(posSB, &SB) == FALLO) return FALLO;
    printf("SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);
    liberar_bloque(nbloque);

    // Leer SB actualizado otra vez
    if(bread(posSB, &SB) == FALLO) return FALLO;
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);

    return EXITO;
}

int mostrar_inodo_raiz() {
    struct inodo inodo;
    char atime[80], mtime[80], ctime[80], btime[80];
    struct tm *ts;

    printf("\nDATOS DEL DIRECTORIO RAIZ\n");

    if (leer_inodo(SB.posInodoRaiz, &inodo) == FALLO) return FALLO;

    //formatear hora (copiado y pegado de adelaida)
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&inodo.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);

    //prints
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %u\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("btime: %s\n", btime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);

    return EXITO;
}

int mostrar_inodo(int ninodo) {
    struct inodo inodo;
    char atime[80], mtime[80], ctime[80], btime[80];
    struct tm *ts;

    printf("\nDATOS DEL INODO RESERVADO %u\n", ninodo);

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    //formatear hora (copiado y pegado de adelaida)
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&inodo.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);

    //prints
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %u\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("btime: %s\n", btime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);

    return EXITO;
}

int pruebaNivel4(){
    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n\n");
    int ninodo = reservar_inodo('f', 6); 

    traducir_bloque_inodo(ninodo, 8, 1);
    traducir_bloque_inodo(ninodo, 204, 1);
    traducir_bloque_inodo(ninodo, 30004, 1);
    traducir_bloque_inodo(ninodo, 400004, 1);
    traducir_bloque_inodo(ninodo, 468750, 1);

    mostrar_inodo(ninodo);
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

    // Print de niveles

    #if NIVEL2
        leer_superbloque();
        print_inodos();
    #endif

    #if NIVEL3
        leer_superbloque();
        prueba_reservar_liberar_bloque();
        mostrar_mapa_bits();
        mostrar_inodo_raiz();
    #endif

    #if NIVEL4
        leer_superbloque();
        pruebaNivel4();
        if(bread(posSB, &SB) == FALLO) return FALLO;
        printf("\nSB.posPrimerInodoLibre = %u\n", SB.posPrimerInodoLibre);
    #endif

    bumount();
    return EXITO;
}