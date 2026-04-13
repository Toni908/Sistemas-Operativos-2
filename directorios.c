//Antonio García Font y Maria Isabel Herrero Soteras  
#include "directorios.h"

#define NIVEL7 1

#define DEBUG 1

// esto no esta nada bien (creo)
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    // Comprobar que el camino empieza por '/'
    if (camino[0] != '/') {
        return -1; // ERROR
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

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){

    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    struct entrada buffer[BLOCKSIZE / sizeof(struct entrada)]; //array de las entradas que caben en un bloque
    unsigned char inicial[sizeof(entrada.nombre)];
    unsigned char final[strlen(camino_parcial)];
    unsigned char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;
    int offset = 0;

    bread(posSB, &SB);

    if(camino_parcial == "/"){ //si es el directorio raiz
        *p_inodo = SB.posInodoRaiz; //nuestra raiz siempre estara asociada con el inodo 0
        *p_entrada = 0;
        return EXITO;
    }

    if(extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO){
        return ERROR_CAMINO_INCORRECTO;
    }

    //buscamos la entrada cuyo nombre se encuentra en el inicial
    if(leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO){
        return ERROR_PERMISO_LECTURA;
    }

    memset(buffer, 0, sizeof(buffer));
    
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada); //Calculamos la cantidad de entradas que tiene el inodo
    num_entrada_inodo = 0; //numero de entradas inicial

    if(cant_entradas_inodo > 0){
        
        offset += mi_read_f(*p_inodo_dir, buffer, offset, BLOCKSIZE); //leemos la entrada
        while((num_entrada_inodo < cant_entradas_inodo) && (inicial != entrada.nombre)){
            num_entrada_inodo++;
            offset += mi_read_f(*p_inodo_dir, buffer, offset, BLOCKSIZE); //leemos la siguiente entrada
        }
    }

    if((inicial != entrada.nombre) && (num_entrada_inodo == cant_entradas_inodo)){
        if(reservar == 0){
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        }else if(reservar == 1){
            if(inodo_dir.tipo == 'f'){
                return ERROR_PERMISO_LECTURA;
            }else{
                //CONTINUAR PROGRAMANDO A PARTIR DE AQUI
            }
        }

    }


}

//Funcion para mostrar los errores
void mostrar_error_buscar_entrada(int error) {
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
