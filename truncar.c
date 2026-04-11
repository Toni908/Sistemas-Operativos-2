//Antonio García Font y Maria Isabel Herrero Soteras
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

int main(int argc, char **argv){
     
    //Comprobamos la sintaxis
    if(argc != 4){
        fprintf(stderr, RED "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n" RESET);
        fprintf(stderr, "  nbytes = 0 : libera el inodo completo\n");
        fprintf(stderr, "  nbytes > 0 : trunca el fichero a nbytes bytes\n");
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ //Montamos el dispositivo virtual
        return FALLO;
    }

    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);
    int resultado;
    
    if(nbytes == 0){ //si nbytes es igual a 0, liberamos el indodo
        resultado = liberar_inodo(ninodo);
        if(resultado == FALLO){
            fprintf(stderr, RED "Error al liberar el inodo %d\n" RESET, ninodo);
        }
    } else { //sino lo truncamos
        resultado = mi_truncar_f(ninodo, nbytes);
        if(resultado == FALLO){
            fprintf(stderr, RED "Error al truncar el inodo %d a %d bytes\n" RESET, ninodo, nbytes);
        }
    }

    // No usamos el de mi_mkfs, porque cambia el titulo y no se muy bien que hacer para hacerlo bien
    struct STAT stat;
    if(mi_stat_f(ninodo, &stat) == EXITO){
        printf("\nDATOS INODO %d:\n", ninodo);
        printf("tipo=%c\n", stat.tipo);
        printf("permisos=%d\n", stat.permisos);
        printf("atime: %s", ctime(&stat.atime));
        printf("mtime: %s", ctime(&stat.mtime));
        printf("ctime: %s", ctime(&stat.ctime));
        printf("btime: %s", ctime(&stat.btime));
        printf("nlinks=%d\n", stat.nlinks);
        printf("tamEnBytesLog=%d\n", stat.tamEnBytesLog);
        printf("numBloquesOcupados=%d\n", stat.numBloquesOcupados);
    }

    bumount();
    return EXITO;
}