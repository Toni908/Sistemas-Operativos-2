//Antonio García Font y Maria Isabel Herrero Soteras
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ficheros.h"

#define TAMBUFFER 1500  //parametro modificable

int main(int argc, char **argv) {

    //Comprobamos la sintaxis
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./leer <nombre_dispositivo> <ninodo>\n");
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    unsigned int ninodo = atoi(argv[2]);

    // Montar dispositivo
    if (bmount(nombre_dispositivo) == FALLO) {
        perror("Error al montar el dispositivo");
        return FALLO;
    }

    char buffer[TAMBUFFER];
    int leidos = 0;
    int total_leidos = 0;
    unsigned int offset = 0;

    // Leer bloque a bloque
    memset(buffer, 0, TAMBUFFER);
    leidos = mi_read_f(ninodo, buffer, offset, TAMBUFFER);

    while (leidos > 0) {

        // Escribir en stdout SOLO lo leído
        write(1, buffer, leidos);

        total_leidos += leidos;
        offset += TAMBUFFER;

        memset(buffer, 0, TAMBUFFER);
        leidos = mi_read_f(ninodo, buffer, offset, TAMBUFFER);
    }

    // Obtener metadatos para mostrar tamEnBytesLog
    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == FALLO) {
        bumount();
        return FALLO;
    }

    // Mostrar info por stderr (no contaminar salida) 
    // esto se utiliza para que los stdr no salgan tarde, ya que imprimia pero al final
    // asi que utilizamos el canal de error
    char info[128];
    sprintf(info, RESET "\ntotal_leidos %d\ntamEnBytesLog %d\n",
            total_leidos, stat.tamEnBytesLog);
    write(2, info, strlen(info));
    bumount();

    return 0;
}