#include <time.h>
#include "ficheros_basico.h"
#include <limits.h>

struct STAT {
    unsigned char tipo;       // Tipo ('l','d','f')
    unsigned char permisos;   // Permisos

    unsigned char reservado_alineacion1[6]; // tengo que mirarlo

    time_t atime;  // último acceso
    time_t mtime;  // última modificación contenido
    time_t ctime;  // última modificación del inodo
    time_t btime;  // creación

    unsigned int nlinks;             // enlaces
    unsigned int tamEnBytesLog;      // tamaño lógico
    unsigned int numBloquesOcupados; // bloques ocupados
};

//Nivel 5
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);

//Nivel 6
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);