#include "bloques.h"

static int descriptor = 0;

//Funcion para montar el dipositivo virtual
int bmount(const char *camino){
    descriptor = open(camino, O_RDWR | O_CREAT, 0666); //Abrimos el fichero
    if(descriptor == FALLO){ 
        perror(RED "Error"); 
        return FALLO;
    }
    return descriptor;
}

//Funcion para desmontar el dispotivo viertual
int bumount(){
    //Programar
    return FALLO; //esto solo es para poder programar lo otro
}

//Funcion que sirve para escribir en un bloque
int bwrite(unsigned int nbloque, const void *buf){
    off_t desplazamiento = nbloque * BLOCKSIZE; //calculamos el desplazamiento para saber donde empezar a escribir
    off_t pos = lseek(descriptor, desplazamiento , SEEK_SET); //movemos el puntero del fichero en el offset correto
    if(pos == FALLO){
        perror(RED "Error"); 
        return FALLO;
    }
    size_t bytes = write(descriptor, buf, BLOCKSIZE); //volcamos el contenido del buffer en el dispositivo virtual
    if(bytes == FALLO){
        perror(RED "Error"); 
        return FALLO;
    }
    return bytes;
}

//Funcion para leer un bloque
int bread(unsigned int nbloque, void *buf){
    //Programar
}