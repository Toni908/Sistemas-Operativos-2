#include "directorios.h"


#define NIVEL7 1

#define DEBUG 1

// esto no esta nada bien (creo)
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    // Comprobar que el camino empieza por '/'
    if (camino[0] != '/') {
        return FALLO; // ERROR
    }

    // Si el camino es solo "/"
    if (strcmp(camino, "/") == 0) {
        strcpy(inicial, "/");
        strcpy(final, "");
        *tipo = 'd';
        return 0;
    }

    // Buscar el siguiente '/'
    const char *p = strchr(camino + 1, '/');

    if (p == NULL) {
        // No hay más '/', es el último componente (fichero)
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        *tipo = 'f';
    } else {
        // Hay más niveles (directorio)
        int len = p - (camino + 1);

        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0';

        strcpy(final, p);
        *tipo = 'd';
    }

    return 0;
}

//Funcion que nos buscara una determinada entrada entre las entradas del inodo correspondiente a su directorio padre
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){

    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    unsigned char inicial[sizeof(entrada.nombre)];
    unsigned char final[strlen(camino_parcial)];
    unsigned char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;

    if(camino_parcial == '/'){ //si es el directorio raiz
        *p_inodo = SB.posInodoRaiz; //nuestra raiz siempre estara asociada al inodo 0
        *p_entrada = 0;
        return EXITO;
    }

    if(extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO){
        return ERROR_CAMINO_INCORRECTO;
    }

    //Buscamos la entrada cuyo nombre se encuentra en inicial
    if(leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO){ //NO SE SI ESTO SE GESTIONA ASI
        return ERROR_PERMISO_LECTURA;
    }

    //seguir a partir de aqui
}


//Funcion auxiliar para imprimir los mensages de error correspondientes
void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
   switch (error) {
   case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}