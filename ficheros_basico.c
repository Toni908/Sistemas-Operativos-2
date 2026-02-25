#include "ficheros_basico.h"
#include "bloques.h"

struct superbloque SB;

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB(); 
int initAI();
int escribir_bit(unsigned int nbloque, unsigned int bit);
char leer_bit(unsigned int nbloque);
int reservar_bloque();
int liberar_bloque(unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, struct inodo *inodo);
int leer_inodo(unsigned int ninodo, struct inodo *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);


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

    return EXITO;

}

//Funcion que inicializa la lista de inodos libres
int initAI(){
    struct inodo inodos [BLOCKSIZE / INODOSIZE];
    if (bread(posSB, &SB) == -1) { //leemos el superbloque
        perror(RED "Error"); // lo quitamos?
        return FALLO;
    }
    int contInodos = SB.posPrimerInodoLibre +1;
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
         if (bread(i, inodos) == -1) { //leemos el bloque de inodos i del dispositivo virtual
            perror(RED "Error"); // lo quitamos?
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
            perror(RED "Error"); // lo quitamos?
            return FALLO;
        }
    }
    return EXITO;    
}

//FALTA ACABARLO, CONTROL DE ERRORES Y RETURN
int escribir_bit(unsigned int nbloque, unsigned int bit){
    unsigned char bufferMB[BLOCKSIZE];
    int posbyte = nbloque/8;                           //Dividimos entre 8 porque los bits se agrupan de 8 en 8, asi sabemos la posición del byte
    int posbit = nbloque % 8;                          //Asi sabemos la posicion del bit dentro del byte
    int nbloqueMB = posbyte/BLOCKSIZE;                 //Este es el numero de bloque detro del mapa de bits
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; //Posición absoluta del dispositivo virtual en que se encuentra el bloque
    bread(nbloqueabs,bufferMB);                      
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;                       //Macara para poner el bit a 1
    mascara >>= posbit;                                //Desplazamiento de bits a la derecha
    if(bit == 0){
        bufferMB[posbyte] &= ~mascara;                 //Ponemos a 0 el bit (usar si se quiere poner un 0, liberar bloque)
    }else if(bit == 1){
        bufferMB[posbyte] |= mascara;                  //Ponemos a 1 el bit (usar si se quiere poner un 1, reservar bloque)
    }else{
        printf(RED "Error, el bit tiene que estar a 1 para reservar un bloque o a 0 para liberar un bloque" RESET);
    }
    bwrite(nbloqueabs, bufferMB);                      //Escribimos ese el resultado con ese bit cambiado
    return FALLO;
}

char leer_bit(unsigned int nbloque){
    unsigned char bufferMB[BLOCKSIZE];
    int posbyte = nbloque/8;
    int posbit = nbloque %8;
    int nbloqueMB = posbyte/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    bread(nbloqueabs,bufferMB);                    
    posbyte = posbyte % BLOCKSIZE;       
    unsigned char mascara = 128;
    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7 - posbit); //desplazamiento de bits a la derecha
    if(mascara != 0 && mascara != 1){
        printf("Error, el contenido de la mascara es erroneo");
        return FALLO;
    }
    return mascara;
}

//FALTA TODO
int reservar_bloque(){
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE);
    int nbloqueMB = 0;
    if(SB.cantBloquesLibres != 0){
        for(int i = SB.posPrimerBloqueMB; i < SB.posPrimerBloqueAI; i++){
            bread(SB.posPrimerBloqueMB + nbloqueMB, bufferMB);
            if(memcmp(bufferMB, bufferAux, 1024*8) != 0){
                break;
            }else{
                nbloqueMB ++;
            }
        }
        int posbyteB;
        for(int i = 0; i < BLOCKSIZE; i++){
           if(bufferMB[i] != 255){
                posbyteB = i;
                break;
           }
        }
        unsigned char mascara = 128; // 10000000
        int posbit = 0;
        while (bufferMB[posbyteB] & mascara) { // operador AND para bits
            bufferMB[posbyteB] <<= 1;          // desplazamiento de bits a la izquierda
            posbit++;
        }
        int nbloque = (nbloqueMB * BLOCKSIZE  +  posbyteB) * 8 + posbit;
        escribir_bit(nbloque, 1);        
        SB.cantBloquesLibres --;

        bwrite(posSB, &SB); // guardar SB

        return nbloque;
    }else{
        printf(RED "Error, no quedan bloques libres");
        return FALLO;
    }
}

int liberar_bloque(unsigned int nbloque){
    //PROGRAMAR
    SB.cantBloquesLibres ++;
    return FALLO;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo){
    //PROGRAMAR
    return FALLO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    //PROGRAMAR
    return FALLO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    //PROGRAMAR
    return FALLO;
}

