#include "ficheros_basico.h"

#define NIVEL2 0
#define NIVEL3 0
#define NIVEL4 0
#define NIVEL5 1
#define NIVEL6 1

#define DEBUG 1

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
    struct superbloque SB;

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

    int result = bwrite(0, &SB);

    return result;
}

//Funcion que inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos
int initMB(){
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    unsigned int bloques_metadatos = SB.posPrimerBloqueDatos; // el ultimo bloque de metadatos es posPrimerBloqueDatos -1, por tanto posPrimerBloqueDatos seria 
                                                              // el numero de bloques de metadatos

    unsigned int bloques_completos = bloques_metadatos / (8 * BLOCKSIZE);
    unsigned int resto_bits = bloques_metadatos % (8 * BLOCKSIZE);

    unsigned int posBloqueMB = SB.posPrimerBloqueMB;

    // Bloques del MB completamente a 1
    for(unsigned int i = 0; i < bloques_completos; i++){
        memset(bufferMB, 255, BLOCKSIZE);
        if(bwrite(posBloqueMB + i, bufferMB) == FALLO) return FALLO;
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

        if(bwrite(posBloqueMB + bloques_completos, bufferMB) == FALLO) return FALLO;
    }

    // Actualizar superbloque segun initMB
    SB.cantBloquesLibres -= bloques_metadatos;

    if(bwrite(posSB, &SB) == FALLO) return FALLO;

    return EXITO;

}

//Funcion que inicializa la lista de inodos libres
int initAI(){
    struct superbloque SB;
    struct inodo inodos [BLOCKSIZE / INODOSIZE];
    if (bread(posSB, &SB) == FALLO) return FALLO;

    int contInodos = SB.posPrimerInodoLibre +1;
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
         if (bread(i, inodos) == FALLO) return FALLO;
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
        if (bwrite(i, inodos) == FALLO) return FALLO;
    }
    return EXITO;    
}

int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    unsigned char bufferMB[BLOCKSIZE];
    int posbyteMB = nbloque/8;                           //Dividimos entre 8 porque los bits se agrupan de 8 en 8, asi sabemos la posición del byte
    int posbit = nbloque % 8;                          //Asi sabemos la posicion del bit dentro del byte
    int nbloqueMB = posbyteMB/BLOCKSIZE;                 //Este es el numero de bloque detro del mapa de bits
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; //Posición absoluta del dispositivo virtual en que se encuentra el bloque
    
    if(bread(nbloqueabs,bufferMB) == FALLO) return FALLO;                      
    int posbyte = posbyteMB % BLOCKSIZE;
    unsigned char mascara = 128;                       //Macara para poner el bit a 1
    mascara >>= posbit;                                //Desplazamiento de bits a la derecha
    if(bit == 0){
        bufferMB[posbyte] &= ~mascara;                 //Ponemos a 0 el bit (usar si se quiere poner un 0, liberar bloque)
    }else if(bit == 1){
        bufferMB[posbyte] |= mascara;                  //Ponemos a 1 el bit (usar si se quiere poner un 1, reservar bloque)
    }else{
        printf(RED "Error, el bit tiene que estar a 1 para reservar un bloque o a 0 para liberar un bloque" RESET);
    }
    if(bwrite(nbloqueabs, bufferMB) == FALLO) return FALLO;                      //Escribimos ese el resultado con ese bit cambiado
    
    return EXITO;
}

char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    unsigned char bufferMB[BLOCKSIZE];
    int posbyteMB = nbloque/8;
    int posbit = nbloque %8;
    int nbloqueMB = posbyteMB/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    
    if(bread(nbloqueabs,bufferMB) == FALLO) return FALLO;                    
    int posbyte = posbyteMB % BLOCKSIZE;       
    unsigned char mascara = 128;
    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7 - posbit); //desplazamiento de bits a la derecha

    if(mascara != 0 && mascara != 1){
        printf("Error, el contenido de la mascara es erroneo");
        return FALLO;
    }

    #if DEBUG && NIVEL3
        printf(GRAY "[leer_bit(%d)→ posbyteMB:%d, posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, nbloque, posbyteMB, posbyte, posbit, nbloqueMB, nbloqueabs);
    #endif

    return mascara;
}

int reservar_bloque(){
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE);
    int nbloqueMB = 0;
    if(SB.cantBloquesLibres != 0){
        for(int i = SB.posPrimerBloqueMB; i < SB.posPrimerBloqueAI; i++){
            if(bread(SB.posPrimerBloqueMB + nbloqueMB, bufferMB) == FALLO) return FALLO;
            if(memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0){
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
        if(escribir_bit(nbloque, 1) == FALLO) return FALLO;        
        SB.cantBloquesLibres --;

        if(bwrite(posSB, &SB) == FALLO) return FALLO; // guardar SB

        return nbloque;
    }else{
        printf(RED "Error, no quedan bloques libres");
        return FALLO;
    }
}

int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    if(bread(posSB, &SB) == FALLO) return FALLO;

    if(escribir_bit(nbloque, 0) == FALLO) return FALLO;
    SB.cantBloquesLibres ++;

    if(bwrite(posSB, &SB) == FALLO) return FALLO;  // guardar SB
    return nbloque;  
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo){
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    int nbloqueAI = ninodo * INODOSIZE / BLOCKSIZE;
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;

    if(bread(nbloqueabs, inodos) == FALLO) return FALLO;
    
    int posinodo = ninodo % (BLOCKSIZE/INODOSIZE);
    inodos[posinodo] = *inodo;

    if(bwrite(nbloqueabs, inodos) == FALLO) return FALLO;
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct superbloque SB;
    if(bread(posSB, &SB) == FALLO) return FALLO;

    int nbloqueAI = ninodo * INODOSIZE / BLOCKSIZE;
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
    if(bread(nbloqueabs, inodos) == FALLO) return FALLO;
    int posinodo = ninodo % (BLOCKSIZE/INODOSIZE);
    *inodo = inodos[posinodo];
    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque SB;
    if(bread(posSB, &SB) == FALLO) return FALLO;
    
    if(SB.cantInodosLibres != 0){
        struct inodo inodo;
        struct inodo inodos[BLOCKSIZE/INODOSIZE];
        int nbloqueAI = SB.posPrimerInodoLibre * INODOSIZE / BLOCKSIZE;
        int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
        if(bread(nbloqueabs, inodos) == FALLO) return FALLO;

        int posinodo = SB.posPrimerInodoLibre % (BLOCKSIZE/INODOSIZE);
        struct inodo aux = inodos[posinodo];
        int posInodoReservado = SB.posPrimerInodoLibre;
        SB.posPrimerInodoLibre = aux.punterosDirectos[0];

        inodo.tipo = tipo;
        inodo.permisos = permisos;
        inodo.nlinks = 1;
        inodo.tamEnBytesLog = 0;
        inodo.atime = time(NULL);
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.btime = time(NULL);
        inodo.numBloquesOcupados = 0;
        for(int i = 0; i < sizeof(inodo.punterosDirectos); i++){
            inodo.punterosDirectos[i] = 0;
        }
        for(int i = 0; i < sizeof(inodo.punterosIndirectos); i++){
            inodo.punterosIndirectos[i] = 0;
        }

        if(escribir_inodo(posInodoReservado, &inodo) == FALLO) return FALLO;

        SB.cantInodosLibres--;
        if(bwrite(posSB, &SB) == FALLO) return FALLO; // guardar SB
        return posInodoReservado;
    }else{
        printf(RED "Error, no hay indodos libres" RESET);
        return FALLO;
    }
}

//Devuelve el rango de punteros en el que me encuentro
int obtener_nRangoBL (struct inodo *inodo,unsigned int nblogico, unsigned int *ptr){
    if(nblogico < DIRECTOS){ //Si es menor a 12
        *ptr = inodo -> punterosDirectos[nblogico];
        return 0;
    }else if(nblogico < INDIRECTOS0){ //Si es menor a 268
        *ptr = inodo -> punterosIndirectos[0];
        return 1;
    }else if(nblogico < INDIRECTOS1){ //Si es menor a 65.804
        *ptr = inodo -> punterosIndirectos[1];
        return 2;
    }else if(nblogico < INDIRECTOS2){ //Si es menor a 16.843.020 
        *ptr = inodo -> punterosIndirectos[2];
        return 3;
    }else{
        *ptr = 0;
        printf(RED"Bloque lógico fuera de rango"RESET);
        return FALLO;
    }
}

int obtener_indice (unsigned int nblogico, int nivel_punteros){
    if(nblogico < DIRECTOS){
        return nblogico;
    }else if(nblogico < INDIRECTOS0){
        return nblogico - DIRECTOS;
    }else if(nblogico < INDIRECTOS1){
        if(nivel_punteros == 2){
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }else if(nivel_punteros == 1){
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }else if(nblogico < INDIRECTOS2){
        if(nivel_punteros == 3){
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }else if(nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS; 
        }else if(nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS; 
        }
    }

    return FALLO; 
}

//Dado un bloque logico nos devuelve donde esta a nivel físico
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar){
    struct inodo inodo;
    unsigned int ptr = 0;
    unsigned int ptr_ant = 0;
    unsigned int salvar_inodo = 0;

    int nRangoBL;
    int nivel_punteros;
    int indice;

    unsigned int buffer[NPUNTEROS];

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    if(nRangoBL == FALLO) return FALLO;
    nivel_punteros = nRangoBL;
    // Punetros Directos
    if (nRangoBL == 0){
        if (ptr == 0){
            if (reservar == 0){
                return FALLO;
            }
            ptr = reservar_bloque();
            if(ptr == FALLO) return FALLO;
            inodo.punterosDirectos[nblogico] = ptr;
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL); 
            salvar_inodo = 1;
            #if DEBUG && (NIVEL4 || NIVEL5)
                printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosDirectos[%u] = %u (reservado BF %u para BL %u)]\n" RESET, nblogico, ptr, ptr, nblogico);
            #endif
        }
    }else{ // Casos indirectos
        while (nivel_punteros > 0){
            if (ptr == 0){
                if (reservar == 0){
                    return FALLO;
                }
                ptr = reservar_bloque(); // bloque de punteros
                if(ptr == FALLO) return FALLO;
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL);
                salvar_inodo = 1;

                if (nivel_punteros == nRangoBL){
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                    #if DEBUG && (NIVEL4 || NIVEL5)
                        printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %u (reservado BF %u para punteros_nivel%d)]\n" RESET, nRangoBL-1, ptr, ptr, nivel_punteros);
                    #endif
                }else{
                    buffer[indice] = ptr;
                    bwrite(ptr_ant, buffer);
                    #if DEBUG && (NIVEL4 || NIVEL5)
                        printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %u (reservado BF %u para punteros_nivel%u)]\n" RESET, nivel_punteros+1, indice, ptr, ptr, nivel_punteros);
                    #endif
                }
                memset(buffer, 0, BLOCKSIZE);
            }else{
                if(bread(ptr, buffer) == FALLO) return FALLO;
            }
            indice = obtener_indice(nblogico, nivel_punteros);
            if(indice == FALLO) return FALLO;
            ptr_ant = ptr;
            ptr = buffer[indice];

            nivel_punteros--;
        }
        // Bloque de datos
        if (ptr == 0){
            if (reservar == 0){
                return FALLO;
            }
            ptr = reservar_bloque();
            if(ptr == FALLO) return FALLO;
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            salvar_inodo = 1;

            buffer[indice] = ptr;
            if(bwrite(ptr_ant, buffer) == FALLO) return FALLO;
            
            #if DEBUG && (NIVEL4 || NIVEL5)
                printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel1 [%d] = %u (reservado BF %u para BL %u)]\n" RESET, indice, ptr, ptr, nblogico);
            #endif
        }
    }
    #if DEBUG && NIVEL4
        printf("\n");
    #endif
    if (salvar_inodo){
        if(escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    }

    return ptr;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    unsigned int nivel_punteros = 0;
    unsigned int nBL = primerBL;
    unsigned int ultimoBL = 0; 
    unsigned int ptr = 0;
    int nRangoBL = 0;
    int liberados = 0;
    int eof = 0;

    // esto es para el debug de bwrite y bread
    unsigned int bread_p = 0;
    unsigned int bwrite_p = 0;

    // fichero vacio
    if(inodo->tamEnBytesLog == 0){ 
        return 0; 
    }
    
    if(inodo->tamEnBytesLog % BLOCKSIZE == 0){
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
    
    #if DEBUG && NIVEL6
        printf(GRAY "[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n" RESET, primerBL, ultimoBL);
    #endif
    
    // directos
    nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
    if(nRangoBL == 0){
        liberados = liberar_directos(&nBL, ultimoBL, inodo, &eof);
    }

    // indirectos
    while(!eof){
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        nivel_punteros = nRangoBL;
        liberados = liberados + liberar_indirectos_recursivo(&nBL, primerBL, ultimoBL, inodo, nRangoBL, nivel_punteros, &ptr, &eof, &bread_p, &bwrite_p);
    }
    
    #if DEBUG && NIVEL6
        printf(GRAY "[liberar_bloques_inodo()→ total bloques liberados: %d, total_breads: %d, total_bwrites: %d]\n" RESET, liberados, bread_p, bwrite_p);
    #endif
    
    return liberados;
}

int liberar_directos (unsigned int *nBL, unsigned int ultimoBL, struct inodo *inodo, int *eof){
    int liberados = 0;
    
    while (*nBL < DIRECTOS && !(*eof)) {
        if(inodo->punterosDirectos[*nBL] != 0 ){
            #if DEBUG && NIVEL6
                printf(GRAY "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n" RESET, 
                        inodo->punterosDirectos[*nBL], *nBL);
            #endif
            liberar_bloque(inodo->punterosDirectos[*nBL]);
            inodo->punterosDirectos[*nBL] = 0;
            liberados++;
        }
        
        *nBL = *nBL + 1;
        if(*nBL > ultimoBL){
            *eof = 1;
        }
    }
    
    return liberados;
}

int liberar_indirectos_recursivo(unsigned int *nBL, unsigned int primerBL, unsigned int ultimoBL, struct inodo *inodo, int nRangoBL, unsigned int nivel_punteros, unsigned int *ptr, int *eof, int *bread_p, int *bwrite_p) {
    int liberados = 0;
    int modificado = 0;
    unsigned int bloquePunteros[NPUNTEROS];
    unsigned int bufferCeros[NPUNTEROS] = {0};
    int BLliberado = 0;

    if (*ptr == 0) {
        switch (nRangoBL) {
            case 1:
                *nBL = INDIRECTOS0;
                break;
            case 2:
                *nBL = INDIRECTOS1;
                break;
            case 3:
                *nBL = INDIRECTOS2;
                break;
        }
        #if DEBUG && NIVEL6
            printf( GRAY "[liberar_bloques_inodo()→ Saltamos del BL %d al BL %ld]\n" RESET, 
                            primerBL, *nBL);
        #endif
        bread_p++;
        return liberados;
    }

    int indice_inicial = obtener_indice(*nBL, nivel_punteros);
    if (indice_inicial == 0 || *nBL == primerBL) {
        if (bread(*ptr, bloquePunteros) == FALLO) return FALLO;
    }

    for (int i = indice_inicial; i < NPUNTEROS && !(*eof); i++) {
        if (bloquePunteros[i] != 0) {
            if (nivel_punteros == 1) {
                #if DEBUG && NIVEL6
                    printf(GRAY "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n" RESET, 
                            bloquePunteros[i], *nBL);
                    BLliberado = *nBL;
                #endif
                liberar_bloque(bloquePunteros[i]);
                bloquePunteros[i] = 0;
                modificado = 1;
                liberados++;
                *nBL = *nBL + 1;
            } else {
                unsigned int ptr_antes = bloquePunteros[i];
                liberados = liberados + liberar_indirectos_recursivo(nBL, primerBL, ultimoBL, 
                                        inodo, nRangoBL, nivel_punteros - 1, &bloquePunteros[i], eof, bread_p, bwrite_p);
                if (bloquePunteros[i] != ptr_antes) {
                    modificado = 1;
                }
            }
        } else {
            // Agrupamos saltos consecutivos de entradas a 0
            #if DEBUG && NIVEL6
                int nBL_inicio = *nBL;
            #endif
            
            while (i < NPUNTEROS && bloquePunteros[i] == 0 && !(*eof)) {
                switch (nivel_punteros) {
                    case 1: *nBL = *nBL + 1; break;
                    case 2: *nBL = *nBL + NPUNTEROS; break;
                    case 3: *nBL = *nBL + NPUNTEROS * NPUNTEROS; break;
                }
                if (*nBL > ultimoBL) *eof = 1;
                i++;
            }

            #if DEBUG && NIVEL6
                if (*nBL - 1 >= nBL_inicio && nBL_inicio <= ultimoBL) {
                    printf( GRAY "[liberar_bloques_inodo()→ Saltamos del BL %d al BL %d]\n" RESET, 
                            nBL_inicio, *nBL - 1);
                }
            #endif
            bread_p++;
            // Compensamos el i++ del for
            i--;
            continue;
        }
        
        if (*nBL > ultimoBL) {
            *eof = 1;
        }
    }
    
    if (memcmp(bloquePunteros, bufferCeros, BLOCKSIZE) == 0) {
        #if DEBUG && NIVEL6
            printf(GRAY "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n" RESET, 
                    *ptr, nivel_punteros, BLliberado);
        #endif
        liberar_bloque(*ptr);
        *ptr = 0;
        liberados++;
    } else if (modificado == 1) {
        #if DEBUG && NIVEL6
            printf(GRAY "[liberar_bloques_inodo()→ salvado BF %d de punteros_nivel%d correspondiente al BL %d]\n" RESET, 
                    *ptr, nivel_punteros, BLliberado);
        #endif
        bwrite_p++; // para debugear
        bwrite(*ptr, bloquePunteros);
    }
    
    return liberados;
}

int liberar_inodo(unsigned int ninodo) {
    struct inodo inodo;
    struct superbloque SB;

    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Guardar cantidad de bloques liberados
    int liberados = liberar_bloques_inodo(0, &inodo);
    if (liberados == FALLO) return FALLO;

    // Actualizar campos del inodo
    inodo.tipo = 'l'; // libre
    inodo.tamEnBytesLog = 0;
    inodo.numBloquesOcupados -= liberados;  // Deberia quedar 0

    // Leer superbloque
    if (bread(posSB, &SB) == FALLO) return FALLO;

    // Actualizar lista de inodos libres
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;

    // Actualizar ctime
    inodo.ctime = time(NULL);    

    // Escribir inodo y superbloque
    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    if (bwrite(posSB, &SB) == FALLO) return FALLO;

    #if DEBUG && NIVEL6
        printf(GRAY "[liberar_inodo()→ Tras liberar inodo: primerInodoLibre = %d]\n" RESET, SB.posPrimerInodoLibre);
    #endif

    return ninodo;
}