#include "ficheros_basico.h"
#include "bloques.h"
#include <limits.h>

struct superbloque SB;

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB(); 
int initAI();

//Funcion que calcula el tamaño en bloques necesario para el mapa de bits
int tamMB(unsigned int nbloques){
    int mb = (nbloques/8)/BLOCKSIZE; //calculamos el tamño del mapa de bits
    if(((nbloques/8) % BLOCKSIZE) != 0){ //si el modulo no es 0 sumamos 1
        mb = mb + 1;
    }
    return mb;
}

//Funcion que calcula el tamaño en bloques del array de inodos
int tamAI(unsigned int ninodos){
    int tam = (ninodos * INODOSIZE)/BLOCKSIZE;
    if(((ninodos * INODOSIZE) % BLOCKSIZE) != 0){
        tam = tam + 1;
    }
    return tam;
}

//Funcion que inicializa los datos del superbloque
int initSB(unsigned int nbloques, unsigned int ninodos){
    unsigned char buffer[BLOCKSIZE];

    SB.posPrimerBloqueMB = posSB+tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) -1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;    
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) -1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques -1; 
    SB.posInodoRaiz = 0; 
    SB.posPrimerInodoLibre = 0; // al reservar el inodo raiz valdrá 1
    SB.cantBloquesLibres = nbloques; // al ejecutar initMB() restaremos los metadatos 
    SB.cantInodosLibres = ninodos; // al reservar el inodo raiz restaremos 1    
    SB.totBloques =  nbloques;  
    SB.totInodos =  ninodos; 

    memset(buffer, 1 , BLOCKSIZE);
    int result = bwrite(0, buffer);

    return result;
}

//Funcion que inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos
int initMB(){
    unsigned char bufferMB[BLOCKSIZE];

    unsigned int bloques_metadatos = SB.posPrimerBloqueDatos; // el ultimo bloque de metadatos es posPrimerBloqueDatos -1, por tanto posPrimerBloqueDatos seria 
                                                              // el numero de bloques de metadatos

    unsigned int bloques_completos = bloques_metadatos / (8 * BLOCKSIZE);
    unsigned int resto_bits = bloques_metadatos % (8 * BLOCKSIZE);

    unsigned int posBloqueMB = SB.posPrimerBloqueMB;

    // Bloques del MB completamente a 1
    for(unsigned int i = 0; i < bloques_completos; i++){
        memset(bufferMB, 255, BLOCKSIZE);
        bwrite(posBloqueMB + i, bufferMB);
    }

    // Bloque parcial
    if(resto_bits > 0){

        memset(bufferMB, 0, BLOCKSIZE);

        unsigned int bytes_completos = resto_bits / 8;
        unsigned int bits_restantes = resto_bits % 8;

        for(unsigned int i = 0; i < bytes_completos; i++){
            bufferMB[i] = 255;
        }

        if(bits_restantes > 0){
            bufferMB[bytes_completos] = 255 << (8 - bits_restantes);
        }

        bwrite(posBloqueMB + bloques_completos, bufferMB);
    }

    // Actualizar superbloque segun initMB
    SB.cantBloquesLibres -= bloques_metadatos;

    bwrite(posSB, &SB);

    return 0;

   // int bloques_metadatos = tamSB + tamMB + tamAI;
   // escribir_paquetes_bytes;
   // escribir_escribir_bits_restantes;
   // escribir(bufferMB);
   // SB.cantBloquesLibres -= bloques_metadatos;
   // escribir(SB); 
}

//Funcion que inicializa la lista de inodos libres
int initAI(){
    struct inodo inodos [BLOCKSIZE / INODOSIZE];
    if (bread(posSB, &SB) == -1) { //leemos el superbloque
        perror("Error");
        return FALLO;
    }
    int contInodos = SB.posPrimerInodoLibre +1;
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
         if (bread(i, inodos) == -1) { //leemos el bloque de inodos i del dispositivo virtual
            perror("Error");
            return FALLO;
        }
        for(int j = 0; j <= BLOCKSIZE/INODOSIZE; j++){
            inodos[j].tipo = 'l'; //libre
            if(contInodos < SB.totInodos){
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos ++;
            }else{
               inodos[j].punterosDirectos[0] = UINT_MAX; 
               break;
            }
        }
        if (bwrite(i, inodos) == -1) { //escribimos el bloque en el dispositivo final
            perror("Error escribiendo bloque de inodos");
            return FALLO;
        }
    }
    return EXITO;
}