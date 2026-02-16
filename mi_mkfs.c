#include <stdio.h>
#include "bloques.h"

int main(int argc, char **argv){
    unsigned char buffer[BLOCKSIZE];
    bmount(argv[1]); //creamos el dispositivo virtual
    for(int i = 0; i < atoi(argv[2]); i++){ //bucle for para escribir los bloques
        memset(buffer, 0 , BLOCKSIZE);
        bwrite(i, buffer);
    }
    bunmount();  //desmontamos el dispostivo virtual
}
