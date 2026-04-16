//Antonio García Font y Maria Isabel Herrero Soteras  

#include "directorios.h"

#define NIVEL7 1

#define DEBUG 1

// esto no esta nada bien (creo)
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    char copiaCamino[60];

    // 1. comprobar valido
    if (camino[0] != '/') {
        return ERROR_CAMINO_INCORRECTO;
    }

    // 2. copiar camino
    strcpy(copiaCamino, camino);

    // 3. extraer inicial
    strcpy(inicial, strtok(copiaCamino + 1, "/"));

    // 4. extraer final
    char *resto = strtok(NULL, "");

    // 5. obtener tipo
    if (camino[1 + strlen(inicial)] == '/') {
        strcpy(final, "/");
        if (resto != NULL) strcat(final, resto);
        *tipo = 'd';
        return 1;
    } else {
        if (resto != NULL) strcpy(final, resto);
        *tipo = 'f';
        return 0;
    }
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){

    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    struct entrada buffer[BLOCKSIZE / sizeof(struct entrada)]; //array de las entradas que caben en un bloque
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;
    int offset = 0;

    bread(posSB, &SB);

    if(strcmp(camino_parcial,"/") == 0){ //si es el directorio raiz
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
        if(reservar == 0){ //modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        }else if(reservar == 1){ //Modo escritura. Creamos la entrada en el directorio referenciada por *p_inodo_dir
            if(inodo_dir.tipo == 'f'){ //Si es fichero no permitir escritura
                return ERROR_PERMISO_LECTURA;
            }else{
                strcpy(entrada.nombre, inicial);
                if(tipo == 'd'){
                    if(strcmp(final, "/") == 0){
                        entrada.ninodo = reservar_inodo('d', 6); //reservamos un inodo como directorio y lo asignamos a la entrada
                    }else{ //no es el final de la ruta
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }else{ //es un fichero
                    entrada.ninodo = reservar_inodo('f', 6); //reservamos un inodo como fichero y lo asignamos a la entrada
                }
                if(mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) == FALLO){ //escribimos la entrada en el directorio padre 
                    if(entrada.ninodo != -1){
                        liberar_inodo(entrada.ninodo);
                    }
                    return FALLO;
                }  
            }
        }
    }
    if(strcmp(final, "/") == 0 || tipo == 'f'){
        if((num_entrada_inodo < cant_entradas_inodo) && (reservar = 1)){ //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        //cortamos la recursividad
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    }else{
        *p_inodo_dir = buffer[num_entrada_inodo].ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
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
