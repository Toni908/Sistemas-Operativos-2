#include <stdio.h>
#include "bloques.h"
#include "ficheros_basico.h"

int main(int argc, char **argv){
    unsigned char buffer[BLOCKSIZE];
    bmount(argv[1]); //creamos el dispositivo virtual
    memset(buffer, 0 , BLOCKSIZE); // creo que no hace falta que este fuera, ya que no reasignamos buffer en el for, hay que preguntar a adelaida
    for(int i = 0; i < atoi(argv[2]); i++){ //bucle for para escribir los bloques
        bwrite(i, buffer);
    }
    //añadir funciones aqui 
    bumount();  //desmontamos el dispostivo virtual
}
