#include "bloques.h"

static int descriptor = 0;

//Funcion para montar el dipositivo virtual
int bmount(const char *camino){
    umask(0); //para quitar los permisos default que da linux, y podamos poner de verdad los permisos que queremos, por defecto es 22
    descriptor = open(camino, O_RDWR | O_CREAT, 0666); //Abrimos el fichero
    if(descriptor == FALLO){ 
        perror(RED "Error de mount"); 
        return FALLO;
    }
    return descriptor;
}

//Funcion para desmontar el dispotivo viertual
int bumount(){
    if(close(descriptor) == FALLO){
        perror(RED "Error de umount");
        return FALLO;
    }
    descriptor = EXITO; //para dejarlo limpio
    return descriptor;
}

//Funcion que sirve para escribir en un bloque
int bwrite(unsigned int nbloque, const void *buf){
    off_t desplazamiento = nbloque * BLOCKSIZE; //calculamos el desplazamiento para saber donde empezar a escribir
    off_t pos = lseek(descriptor, desplazamiento , SEEK_SET); //movemos el puntero del fichero en el offset correto
    if(pos == FALLO){
        perror(RED "Error de pos en write"); 
        return FALLO;
    }
    ssize_t bytes = write(descriptor, buf, BLOCKSIZE); //volcamos el contenido del buffer en el dispositivo virtual, es ssize_t segun la doc de c. Una s es para unasigned, nunca sabremos si da error
    if(bytes == FALLO){
        perror(RED "Error de escritura de bytes"); 
        return FALLO;
    }
    return bytes;
}

//Funcion para leer un bloque
int bread(unsigned int nbloque, void *buf){
    off_t desplazamiento = nbloque * BLOCKSIZE; //calculamos el desplazamiento para saber donde empezar a leer
    off_t pos = lseek(descriptor, desplazamiento , SEEK_SET); //movemos el puntero del fichero en el offset correto
    if(pos == FALLO){
        perror(RED "Error de pos en read"); 
        return FALLO;
    }
    ssize_t bytes = read(descriptor, buf, BLOCKSIZE); //leemos el contenido del bloque
    if(bytes == FALLO){
        perror(RED "Error de lectura de bytes"); 
        return FALLO;
    }
    return bytes;
}