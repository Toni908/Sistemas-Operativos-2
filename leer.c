#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ficheros.h"

// para los tests y que de problemitas, adelaida lo comento
// porque si es 1024, es facilito, tenemos que demostrar que con cualquier
// buffer funciona
#define tambuffer 1500 

int main(int argc, char **argv){
    unsigned int ninodo;
    int offset = 0;
    int leidos = 0;
    int total_leidos = 0;
    unsigned char buffer[tambuffer];
    struct STAT stat;

    // Validación de sintaxis
    if(argc != 3){
        fprintf(stderr, RED "Sintaxis: leer <nombre_dispositivo> <ninodo>\n" RESET);
        exit(-1);
    }

    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, RED "Error al montar el dispositivo\n" RESET);
        return FALLO;
    }
    
    ninodo = atoi(argv[2]);
    
    // Limpiar buffer antes de la primera lectura
    memset(buffer, 0, tambuffer);
    leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    
    while(leidos > 0){
        write(1, buffer, leidos);  
        total_leidos += leidos;
        offset += tambuffer;
        
        // Limpiar buffer antes de cada lectura, por esto petaba
        memset(buffer, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    }
    
    // El arbol no me esta dejando ver el bosque?
    if(mi_stat_f(ninodo, &stat) == FALLO){
        fprintf(stderr, RED "Error al obtener estadísticas del inodo\n" RESET);
        bumount();
        return FALLO;
    }
    
    fprintf(stderr, "total_leidos %d\n", total_leidos);
    fprintf(stderr, "tamEnBytesLog %d\n", stat.tamEnBytesLog);
    
    bumount();
    
    return EXITO;
}