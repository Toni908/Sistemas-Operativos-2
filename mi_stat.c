#include "directorios.h"

int main(int argc, char **argv){
    //Comprobamos la sintaxis
    if(argc != 3){
        fprintf(stderr, RED "Sintaxis: ./mi_stat <nombre_dispositivo> </ruta>\n" RESET);
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){
        return FALLO;
    }
    
    struct STAT p_stat;
    int ninodo = mi_stat(argv[2], &p_stat);
    
    if(ninodo < 0){
        mostrar_error_buscar_entrada(ninodo);
        bumount();
        return FALLO;
    }
    
    // imprimimos la info del stat (estoy bastante seguro de que esta es la cuarta vez que hago esta funcion tenemos que ver si podemos generalizar esto)
    // creo que tenemos algo parecido en truncar y en leer_sf, aunque pasamos numero de inodo y lo lee directamente sin el p_stat.
    printf(CYAN "Nº de inodo: %d\n" RESET, ninodo);
    printf("tipo: %c\n", p_stat.tipo);  // 'd' o 'f', no entero
    printf("permisos: %d\n", p_stat.permisos);
    
    // Mostrar tiempos en formato legible
    struct tm *tm_info;
    char tiempo[100];
    
    tm_info = localtime(&p_stat.atime);
    strftime(tiempo, sizeof(tiempo), "%a %Y-%m-%d %H:%M:%S", tm_info);
    printf("atime: %s\n", tiempo);
    
    tm_info = localtime(&p_stat.mtime);
    strftime(tiempo, sizeof(tiempo), "%a %Y-%m-%d %H:%M:%S", tm_info);
    printf("mtime: %s\n", tiempo);
    
    tm_info = localtime(&p_stat.ctime);
    strftime(tiempo, sizeof(tiempo), "%a %Y-%m-%d %H:%M:%S", tm_info);
    printf("ctime: %s\n", tiempo);
    
    tm_info = localtime(&p_stat.btime);
    strftime(tiempo, sizeof(tiempo), "%a %Y-%m-%d %H:%M:%S", tm_info);
    printf("btime: %s\n", tiempo);
    
    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);
    
    bumount();
    return EXITO;
}