#include "bloques.h"

struct superbloque{
    unsigned int posPrimerBloqueMB;                     //Posición absoluta del primer bloque del mapa de bits
    unsigned int posUltimoBloqueMB;                     //Posición absoluta del último bloque del mapa de bits
    unsigned int posPrimerBloqueAI;                     //Posición absoluta del primer bloque del mapa de inodos
    unsigned int posUltimoBloqueAI;                     //Posición absoluta del último bloque del mapa de inodos
    unsigned int posPrimerBloqueDatos;                  //Posición absoluta del primer bloque de datos
    unsigned int posUltimoBloqueDatos;                  //Posición absoluta del último bloque de datos
    unsigned int posInodoRaiz;                          //Posición del inodo del directorio raíz (relativa al AI)
    unsigned int posPrimerInodoLibre;                   //Posición del primer inodo libre (relativa al AI)
    unsigned int cantBloquesLibres;                     //Cantidad de bloques libres (en todo el disco)
    unsigned int cantInodosLibres;                      //Cantidad de inodos libres (en el AI)
    unsigned int totBloques;                            //Cantidad total de bloques del disco
    unsigned int totInodos;                             //Cantidad total de inoddos (heuristica)
    char padding[BLOCKSIZE - 12*sizeof(unsigned int)];  //Relleno para ocupar el bloque completo
}