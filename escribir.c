#include <stdio.h>
#include "ficheros.h"
#include <unistd.h>  // Para la llamada al sistema write() //BORRAR

int main(int argc, char **argv){
    int  offsets [] = {9000, 209000, 30725000,409605000,480000000};
    int num_offsets = sizeof(offsets) / sizeof(offsets[0]);
    struct  STAT stat;
    
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        fprintf(stderr, RED "Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        fprintf(stderr,"Offsets: ");
        for(int i = 0; i < num_offsets; i++){
            fprintf(stderr,"%d ", offsets[i]);
        }
        fprintf(stderr,"\n");
        fprintf(stderr,"Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n" RESET);
        exit(-1);
    }
    
    if(bmount(argv[1]) == FALLO){ // Disco virtual
        return FALLO;
    }

    printf("Longitud del texto %ld\n", strlen(argv[2]));

    if(atoi(argv[3]) == 0){ //caso de que se reserva un inodo para todos los offsets
        unsigned int ninodo = reservar_inodo('f',6);
        for(int i = 0; i < num_offsets; i++){
            printf("Numero de inodo reservado: 1\n");
            printf("Offset: %d\n", offsets[i]);
            int bytes_escritos = mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));
            // ==========================================================
            // --- INICIO CÓDIGO DE PRUEBA ---
            // ==========================================================
            int longitud = strlen(argv[2]); // <-- AQUÍ DEFINIMOS LONGITUD
            char buffer_test[longitud];
            memset(buffer_test, 0, longitud);
            int bytes_leidos = mi_read_f(ninodo, buffer_test, offsets[i], longitud);
            // 2. Comprobamos la lectura
            printf("--- DEBUG --- bytes_leidos: %d\n", bytes_escritos);
            fflush(stdout);
            write(1, buffer_test, bytes_leidos);
            printf("\n"); // Salto de línea
            // ==========================================================
            // --- FIN CÓDIGO DE PRUEBA ---
            // ==========================================================
            mi_stat_f(ninodo, &stat);
            printf("Bytes escritos: %ld\n", strlen(argv[2]));
            printf("stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados = %d\n", stat.numBloquesOcupados);
        }
    }else{
        for(int i = 0; i < num_offsets; i++){
            printf("Numero de inodo reservado: %d\n", i+1);
            printf("Offset: %d\n", offsets[i]);
            unsigned int ninodo = reservar_inodo('f',6);
            mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));
            mi_stat_f(ninodo, &stat);
            printf("Bytes escritos: %ld\n", strlen(argv[2]));
            printf("stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados = %d\n", stat.numBloquesOcupados);
        }
    }
    bumount();  //desmontamos el dispostivo virtual

    return EXITO;
}
//hay un fallo con el offset 480000000 imprime uno de mas
//falla en el stat de numero de bloques ocupados
//realmente como se cual es numero de inodo reservado
//fallo tocho leyendo, no lee. :(