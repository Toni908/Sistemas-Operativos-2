//Antonio García Font y Maria Isabel Herrero Soteras  
#include "ficheros.h"

#define NIVEL5 1
#define NIVEL6 0

#define DEBUG 1

//Funcion que escribe el contenido procedente de un buffer de memoria en un fichero/directorio
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int primerBL, ultimoBL;
    unsigned int desp1, desp2;
    int bytes_escritos = 0;

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // comprobar permiso escritura
    if ((inodo.permisos & 2) != 2){
        printf(RED "Error: no hay permiso de escritura\n" RESET);
        return FALLO;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    int bf; // Blque fisico

    // Caso 1: todo en el mismo bloque
    if (primerBL == ultimoBL){
        bf = traducir_bloque_inodo(ninodo, primerBL, 1);
        if(bf == FALLO) return FALLO;
        bread(bf, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, nbytes); // destino origen bytes
        bwrite(bf, buf_bloque);
        bytes_escritos = nbytes;
    } 
    else { // Caso 2:
        // Primer Bloque (1024 - desp1 B)
        bf = traducir_bloque_inodo(ninodo, primerBL, 1);
        if(bf == FALLO) return FALLO;
        bread(bf, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        bwrite(bf, buf_bloque);
        bytes_escritos += BLOCKSIZE - desp1;

        // Bloques Intermedios (1024B)
        for (unsigned int i = primerBL + 1; i < ultimoBL; i++){
            bf = traducir_bloque_inodo(ninodo, i, 1);
            if(bf == FALLO) return FALLO;
            bwrite(bf, buf_original + bytes_escritos);
            bytes_escritos += BLOCKSIZE;
        }

        // Ultimo Bloque (despl2 +1 B, +1 porque se escribe el 0)
        bf = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if(bf == FALLO) return FALLO;
        bread(bf, buf_bloque);
        memcpy(buf_bloque, buf_original + bytes_escritos, desp2 + 1);
        bwrite(bf, buf_bloque);
        bytes_escritos += desp2 + 1;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO; // volver a leer inodo para bloques modificados

    if (offset + nbytes > inodo.tamEnBytesLog){
        inodo.tamEnBytesLog = offset + nbytes;
    }

    inodo.ctime = time(NULL); // ultima data de update del inodo
    inodo.mtime = time(NULL); // ultima data de update de los datos del inodo
    if(escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    return bytes_escritos;
}

//Funcion que lee informacion de un fichero/directorio y la almacena en un buffer de memoria
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){ 
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int primerBL, ultimoBL;
    unsigned int desp1, desp2;
    int bytes_leidos = 0;

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;
    // comprobar permiso lectura
    if ((inodo.permisos & 4) != 4){
        printf(RED "Error: no hay permiso de lectura\n" RESET);
        return FALLO;
    }
    // si offset fuera del fichero
    if (offset >= inodo.tamEnBytesLog){
        return 0;
    }
    // ajustar lectura si supera EOF
    if (offset + nbytes > inodo.tamEnBytesLog){
        nbytes = inodo.tamEnBytesLog - offset;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    int bf; // bloque fisico

    // Caso 1
    if (primerBL == ultimoBL){
        bf = traducir_bloque_inodo(ninodo, primerBL, 0);
        if(bf != FALLO){
            bread(bf, buf_bloque);
            memcpy(buf_original, buf_bloque + desp1, nbytes);  // destino origen bytes
        }
        bytes_leidos = nbytes;
    } 
    else { // Caso 2
        // Primer Bloque
        bf = traducir_bloque_inodo(ninodo, primerBL, 0);
        if(bf != FALLO){
            bread(bf, buf_bloque);
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        bytes_leidos += BLOCKSIZE - desp1;

        // Bloque Intermedio
        for (unsigned int i = primerBL + 1; i < ultimoBL; i++){
            bf = traducir_bloque_inodo(ninodo, i, 0);
            if(bf != FALLO){
                bread(bf, buf_bloque);
                memcpy(buf_original + bytes_leidos, buf_bloque, BLOCKSIZE);
            }
            bytes_leidos += BLOCKSIZE;
        }
        // Ultimo Bloque
        bf = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if(bf != FALLO){
            bread(bf, buf_bloque);
            memcpy(buf_original + bytes_leidos, buf_bloque, desp2 + 1);
        }
        bytes_leidos += desp2 + 1; // incluimos el 0
    }
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO; // volver a leer inodo para bloques modificados

    inodo.atime = time(NULL);
    if(escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;
    return bytes_leidos;
}

//Funcion que devuelve la metainformacion de un fichero/directorio
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;
    
    p_stat->tipo = inodo.tipo;          //tipo de inodo
    p_stat->permisos = inodo.permisos;  //permisos

    p_stat->atime = inodo.atime;        //timestamps
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->btime = inodo.btime;

    p_stat->nlinks = inodo.nlinks;                          //cantidad de enlaces de entradas en directorio      
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;            //tamaño en bytes lógicos
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;  //cantidad de bloques ocupados en la zona de datos

    return EXITO;
}

//Funcion que cambia los permisos de un directorio/fichero
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);    // actualizar cambio de metadatos
    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;

    return EXITO;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    unsigned int primerBL = 0;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    if ((inodo.permisos & 2) != 2) { //En primer lugar comprobamos que el inodo tenga permisos de escritura
       fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
       return FALLO;
    }

    if(nbytes > inodo.tamEnBytesLog){ //Esto es porque no se puede truncar mas alla del tamaño en bytes logico del fichero/directorio
        return FALLO;
    }
 
    if(nbytes % BLOCKSIZE == 0){ //Calculamos el primerBL
        primerBL =  nbytes/BLOCKSIZE;
    }else{
        primerBL =  nbytes/BLOCKSIZE + 1;
    }
    int liberados = liberar_bloques_inodo(primerBL, &inodo);
    inodo.atime = time(NULL); //Actualizamos atime 
    inodo.ctime = time(NULL); //Actualizamos ctime
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;
    escribir_inodo(ninodo, &inodo); //guardamos el inodo
    
    return liberados;
}