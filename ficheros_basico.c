#include "ficheros_basico.h"

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

    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) -1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI +1;
    SB.posUltimoBloqueDatos = nbloques - 1;

    //falta llamar a bwrite en la posicion 0 de la estrcutura del superbloque 
}

//Funcion que inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos
int initMB(){
    unsigned char bufferMB[BLOCKSIZE];
    unsigned int bloques_ocupados = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);
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
    //...
    int contInodos = SB.posPrimerInodoLibre +1;
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
        //leer bloque de inodos i en el dispositivo virtual
        for(int j = 0; j <=     BLOCKSIZE/INODOSIZE; j++){
            inodos[j].tipo = 'l'; //libre
            if(contInodos < SB.totInodos){
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos ++;
            }else{
               // inodos[j].punterosDirectos[0] = UINT_MAX;
            
            }
        }
        
    }
    //ESCRIBIR EL BLOQUE DE INODOS I EN EL DISPOSITIVO FINAL

}