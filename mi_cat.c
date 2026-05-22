//Antonio García Font y Maria Isabel Herrero Soteras  
#include "directorios.h"

#define TAMBUFFERCAT (BLOCKSIZE * 4)  //parametro modificable

int main(int argc, char **argv){

    // Comprobamos sintaxis
    if (argc != 3){
        fprintf(stderr, RED "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }

    // Comprobamos que no sea un directorio
    if (argv[2][strlen(argv[2]) - 1] == '/'){
        fprintf(stderr, RED "Error: la ruta se corresponde a un directorio\n" RESET);
        return FALLO;
    }

    // Montar disco
    if (bmount(argv[1]) == FALLO){
        return FALLO;
    }

    char buffer[TAMBUFFERCAT];
    int leidos;
    unsigned int offset = 0;
    int total_leidos = 0;

    // Lectura secuencial
    while ((leidos = mi_read(argv[2], buffer, offset, TAMBUFFERCAT)) > 0){
        write(1, buffer, leidos);
        offset += leidos;
        total_leidos += leidos;
    }

    // Solo añadir \n si la salida va a terminal, no a archivo
    // Esto es una liada, porque cuando sacamos a terminal, se nos queda pegado el $ del terminal lo que imprime el buffer asi que hay que hacer el salto, 
    // pero solo podemos usarlo a la hora de imprimir y no de guardar, ya que si lo usamos al guardar archivos con > nos añade un byte mas, 
    // encontre esto por internet, y parece que funciona, asi que lo dejare
    if (isatty(1)) {
        write(1, "\n", 1);
    }

    // Mostrar total de bytes leídos
    fprintf(stderr, "Total_leidos %d\n", total_leidos);

    // Desmontar
    bumount();

    return EXITO;
}