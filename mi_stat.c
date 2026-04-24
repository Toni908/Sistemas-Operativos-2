#include "directorios.h"

int main(int argc, char **argv){
    //Comprobamos la sintaxis
    if(argc != 2){
        fprintf(stderr, RED "Poner mensaje adecuado de error" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }
    struct STAT p_stat;
    int ninodo = mi_stat(argv[2], &p_stat);
    
    //imprimimos la info del stat
    printf(CYAN "Numero de inodo: %d\n" RESET, ninodo);
    printf("tipo: %d\n", p_stat.tipo);
    printf("permisos: %d", p_stat.permisos);
    printf("atime: %ld\n", p_stat.atime);
    printf("mtime: %ld\n", p_stat.mtime);
    printf("ctime: %ld\n", p_stat.ctime);
    printf("btime: %ld\n", p_stat.btime);
    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);
    
    bumount();
}