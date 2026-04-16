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

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, 
                   unsigned int *p_inodo, unsigned int *p_entrada, 
                   char reservar, unsigned char permisos) {
    
    char inicial[TAMNOMBRE];
    char final[strlen(camino_parcial) + 1];
    char tipo;
    struct inodo inodo_dir;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    int cant_entradas_inodo, num_entrada_inodo = 0;
    int error;
    
    // Caso: directorio raíz
    if (strcmp(camino_parcial, "/") == 0) {
        *p_inodo = 0;  // SB.posInodoRaiz normalmente es 0
        *p_entrada = 0;
        return 0;  // EXITO
    }
    
    // Extraer camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1) {
        return ERROR_CAMINO_INCORRECTO;
    }
    
    #if DEBUG
    printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", 
           inicial, final, reservar);
    #endif
    
    // Leer el inodo del directorio padre
    if (mi_read_f(*p_inodo_dir, &inodo_dir, 0, sizeof(struct inodo)) == -1) {
        return ERROR_PERMISO_LECTURA;
    }
    
    // Comprobar permisos de lectura (bit 2)
    if (!(inodo_dir.permisos & 4) && reservar == 0) {
        return ERROR_PERMISO_LECTURA;
    }
    
    // Comprobar permisos de escritura (bit 1) si vamos a crear
    if (!(inodo_dir.permisos & 2) && reservar == 1) {
        return ERROR_PERMISO_ESCRITURA;
    }
    
    // Calcular cuántas entradas caben en un bloque
    cant_entradas_inodo = BLOCKSIZE / sizeof(struct entrada);
    
    // Buscar la entrada en los bloques del directorio
    int offset = 0;
    int encontrado = 0;
    unsigned int ninodo_encontrado = 0;
    unsigned int num_entrada_encontrada = 0;
    
    while (offset < inodo_dir.tamEnBytesLog && !encontrado) {
        // Leer un bloque de entradas
        if (mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE) == -1) {
            return ERROR_PERMISO_LECTURA;
        }
        
        // Buscar en las entradas del bloque
        for (int i = 0; i < cant_entradas_inodo && offset + i * sizeof(struct entrada) < inodo_dir.tamEnBytesLog; i++) {
            if (strcmp(entradas[i].nombre, inicial) == 0) {
                encontrado = 1;
                ninodo_encontrado = entradas[i].ninodo;
                num_entrada_encontrada = num_entrada_inodo + i;
                break;
            }
        }
        
        num_entrada_inodo += cant_entradas_inodo;
        offset += BLOCKSIZE;
    }
    
    // Si hemos llegado al final del camino
    if (strcmp(final, "") == 0 || strcmp(final, "/") == 0) {
        if (encontrado && reservar == 1) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        
        if (!encontrado && reservar == 0) {
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        }
        
        if (!encontrado && reservar == 1) {
            // Crear nueva entrada
            if (tipo == 'd') {
                // Reservar inodo para directorio
                if (reservar_inodo('d', permisos) == -1) {
                    return -1;
                }
                *p_inodo = reservar_inodo('d', permisos);
            } else {
                // Reservar inodo para fichero
                if (reservar_inodo('f', permisos) == -1) {
                    return -1;
                }
                *p_inodo = reservar_inodo('f', permisos);
            }
            
            // Añadir entrada al final del directorio
            struct entrada nueva_entrada;
            strcpy(nueva_entrada.nombre, inicial);
            nueva_entrada.ninodo = *p_inodo;
            
            // Escribir la nueva entrada
            int pos_escritura = inodo_dir.tamEnBytesLog;
            if (mi_write_f(*p_inodo_dir, &nueva_entrada, pos_escritura, sizeof(struct entrada)) == -1) {
                return -1;
            }
            
            *p_entrada = num_entrada_inodo;
            
            #if DEBUG
            printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", 
                   *p_inodo, tipo, permisos, inicial);
            printf("[buscar_entrada()→ creada entrada: %s, %d]\n", inicial, *p_inodo);
            #endif
        }
        
        *p_entrada = num_entrada_encontrada;
        return 0;  // EXITO
    }
    
    // No hemos llegado al final, continuar recursivamente
    if (!encontrado) {
        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
    }
    
    // Verificar que el inodo encontrado es un directorio
    struct inodo inodo_hijo;
    if (mi_read_f(ninodo_encontrado, &inodo_hijo, 0, sizeof(struct inodo)) == -1) {
        return -1;
    }
    
    if (inodo_hijo.tipo != 'd') {
        return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
    }
    
    // Llamada recursiva
    *p_inodo_dir = ninodo_encontrado;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
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
