//Antonio García Font y Maria Isabel Herrero Soteras  

#include "directorios.h"

#define NIVEL7 0
#define NIVEL8 0
#define NIVEL9 0
#define NIVEL10 1

#define DEBUG 1

// Variables para USARCACHE 1
#if (USARCACHE == 1)
    static struct UltimaEntrada UltimaEntradaEscritura;
    static struct UltimaEntrada UltimaEntradaLectura;
#endif

// Variables para USARCACHE 2 o 3 (Nivel 9 Tabla)
#if (USARCACHE > 1)
    static struct UltimaEntrada UltimasEntradas[CACHE_SIZE];
#endif

// Variable específica para FIFO
#if (USARCACHE == 2)
    static int ultima_posicion_insertada = 0; 
#endif

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // 1. comprobar válido
    if (camino[0] != '/') {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Caso raíz
    if (strcmp(camino, "/") == 0) {
        strcpy(inicial, "");
        strcpy(final, "");
        *tipo = 'd';
        return EXITO;
    }

    // Buscar la siguiente barra
    const char *p = strchr(camino + 1, '/');

    if (p == NULL) {
        // No hay más barras → es fichero
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        *tipo = 'f';
    } else {
        // Hay más niveles → directorio
        int len = p - (camino + 1);

        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0';

        strcpy(final, p);  // incluye '/'
        *tipo = 'd';
    }

    return EXITO;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {

    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;

    char inicial[TAMNOMBRE];
    char final[strlen(camino_parcial) + 1];
    char tipo;

    int cant_entradas_inodo;
    int num_entrada_inodo = 0;

    int entradas_por_bloque = BLOCKSIZE / sizeof(struct entrada);
    struct entrada buffer_lectura[entradas_por_bloque];

    if(bread(posSB, &SB) == FALLO) return FALLO;

    // Caso raíz
    if (strcmp(camino_parcial, "/") == 0) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0; // no se usara, valor dummy
        return EXITO;
    }

    // Extraer camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0) return ERROR_CAMINO_INCORRECTO;

    #if (DEBUG && (NIVEL7 || NIVEL8)) 
        printf(GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
    #endif

    // Leer inodo del directorio
    if (leer_inodo(*p_inodo_dir, &inodo_dir) < 0) return ERROR_PERMISO_LECTURA;


    // Comprobar permisos de lectura
    if (!(inodo_dir.permisos & 4)) {
        #if (DEBUG && (NIVEL7 || NIVEL8)) 
            printf(GRAY "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }
    
    // Calcular número de entradas, pa saber hasta donde iterar
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    // Buscar entrada
    if (cant_entradas_inodo > 0) {
        memset(buffer_lectura, 0, sizeof(buffer_lectura));

        // Leemos el primer bloque antes del while
        if (mi_read_f(*p_inodo_dir, buffer_lectura, 0, sizeof(buffer_lectura)) < 0) return FALLO;
        entrada = buffer_lectura[0];

        // Mientras no lleguemos al final y no hayamos encontrado el nombre
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0)) {
            num_entrada_inodo++;
            
            // Si tras incrementar aún estamos dentro de los límites, cargamos la siguiente entrada
            if (num_entrada_inodo < cant_entradas_inodo) {
                // Si el nuevo índice es múltiplo de entradas_por_bloque, leemos el siguiente bloque
                if (num_entrada_inodo % entradas_por_bloque == 0) {
                    memset(buffer_lectura, 0, sizeof(buffer_lectura));
                    if (mi_read_f(*p_inodo_dir, buffer_lectura, num_entrada_inodo * sizeof(struct entrada), sizeof(buffer_lectura)) < 0) {
                        return FALLO;
                    }
                }
                // Actualizamos la entrada actual desde el buffer
                entrada = buffer_lectura[num_entrada_inodo % entradas_por_bloque];
            }
        }
    }

    // Si no existe el inodo y hay que crearlo
    if (num_entrada_inodo == cant_entradas_inodo) {
        if (reservar == 0) return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

        // No se puede crear dentro de fichero
        if (inodo_dir.tipo == 'f') return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

        // Permiso escritura
        if (!(inodo_dir.permisos & 2)) return ERROR_PERMISO_ESCRITURA;

        // Crear entrada
        strcpy(entrada.nombre, inicial);

        if (tipo == 'd') {
            if (strcmp(final, "/") != 0) {
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            }
            mi_waitSem();
            entrada.ninodo = reservar_inodo('d', permisos);
            mi_signalSem();

            #if (DEBUG && (NIVEL7 || NIVEL8))
                printf(GRAY "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n" RESET, entrada.ninodo, permisos, inicial);
            #endif
        } else {
            mi_waitSem();
            entrada.ninodo = reservar_inodo('f', permisos);
            mi_signalSem();
            #if (DEBUG && (NIVEL7 || NIVEL8))
                printf(GRAY "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n" RESET, entrada.ninodo, permisos, inicial);
            #endif
        }

        if (entrada.ninodo < 0) return FALLO;

        // Escribir entrada al final
        if (mi_write_f(*p_inodo_dir, &entrada, cant_entradas_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            mi_waitSem();
            liberar_inodo(entrada.ninodo);
            mi_signalSem();
            return FALLO;
        }

        #if (DEBUG && (NIVEL7 || NIVEL8))
            printf(GRAY "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, entrada.nombre, entrada.ninodo);
        #endif
    }

    // Leer entrada encontrada (si ya existía)
    if (num_entrada_inodo < cant_entradas_inodo) {
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            return FALLO;
        }
    }

    // Caso final
    if (strcmp(final, "/") == 0 || tipo == 'f') {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;

        return EXITO;
    }

    // Recursividad
    *p_inodo_dir = entrada.ninodo;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}



//Funcion para mostrar los errores
void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case ERROR_CAMINO_INCORRECTO: fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET); break;
   case ERROR_PERMISO_LECTURA: fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET); break;
   case ERROR_NO_EXISTE_ENTRADA_CONSULTA: fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET); break;
   case ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO: fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET); break;
   case ERROR_PERMISO_ESCRITURA: fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET); break;
   case ERROR_ENTRADA_YA_EXISTENTE: fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET); break;
   case ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO: fprintf(stderr, RED "Error: No es un directorio.\n" RESET); break;
   }
}

//La función, símplemente, hace de wrapper de la función buscar_entrada()
int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    
    if(error < 0){
        mi_signalSem();
        return error;  
    }
    mi_signalSem();
    return EXITO;
}

// La funcion pone el contenido del directorio/fichero en un buffer de memoria
int mi_dir(const char *camino, char *buffer, char tipo, char flag) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    struct inodo inodo;
    struct entrada entrada;
    struct tm *tm_info;
    char tmp[TAMFILA];
    char tmp_nombre[TAMNOMBRE + 30]; // Espacio extra para los códigos de colores

    int error;
    buffer[0] = '\0';

    // Buscar entrada
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0) {
        return error;
    }

    // Leer inodo
    if (leer_inodo(p_inodo, &inodo) < 0) {
        return FALLO;
    }

    // Comprobar permisos de lectura
    if (!(inodo.permisos & 4)) {
        return ERROR_PERMISO_LECTURA;
    }

    // Si es listado extendido, añadimos la cabecera al principio
    if (flag == 'l') {
        strcat(buffer, "Tipo\tPermisos\tmTime\t\tTamaño\tNombre\n");
        strcat(buffer, "----------------------------------------------------------------\n");
    }

    // caso fichero
    if (inodo.tipo == 'f') {
        if (flag == 'l') {
            sprintf(tmp, "%c\t", inodo.tipo);
            strcat(buffer, tmp);

            // Permisos
            strcat(buffer, (inodo.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo.permisos & 1) ? "x\t" : "-\t");

            // Fecha (mTime)
            tm_info = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                    tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                    tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
            strcat(buffer, tmp);

            // Tamaño
            sprintf(tmp, "%d\t", inodo.tamEnBytesLog);
            strcat(buffer, tmp);

            // Nombre
            sprintf(tmp_nombre, CYAN "%s" RESET "\n", strrchr(camino, '/') + 1);
            strcat(buffer, tmp_nombre);
        } else {
            // Listado simple
            sprintf(tmp_nombre, CYAN "%s" RESET "\t", strrchr(camino, '/') + 1);
            strcat(buffer, tmp_nombre);
        }
        return 1; // Un solo fichero listado
    }

    // directorio: hay que listar su contenido
    int cant_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    struct inodo inodo_aux;

    for (int i = 0; i < cant_entradas; i++) {
        if (mi_read_f(p_inodo, &entrada, i * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            return FALLO;
        }

        if (leer_inodo(entrada.ninodo, &inodo_aux) < 0) {
            return FALLO;
        }

        if (flag == 'l') {
            // Tipo
            sprintf(tmp, "%c\t", inodo_aux.tipo);
            strcat(buffer, tmp);

            // Permisos
            strcat(buffer, (inodo_aux.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo_aux.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo_aux.permisos & 1) ? "x\t" : "-\t");

            // Fecha
            tm_info = localtime(&inodo_aux.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                    tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                    tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
            strcat(buffer, tmp);

            // Tamaño
            sprintf(tmp, "%d\t", inodo_aux.tamEnBytesLog);
            strcat(buffer, tmp);

            // Nombres
            if (inodo_aux.tipo == 'd') {
                sprintf(tmp_nombre, RED "%s" RESET "\n", entrada.nombre);
            } else {
                sprintf(tmp_nombre, CYAN "%s" RESET "\n", entrada.nombre);
            }
            strcat(buffer, tmp_nombre);
        } else {
            // Listado simple
            if (inodo_aux.tipo == 'd') {
                sprintf(tmp_nombre, RED "%s" RESET "\t", entrada.nombre);
            } else {
                sprintf(tmp_nombre, CYAN "%s" RESET "\t", entrada.nombre);
            }
            strcat(buffer, tmp_nombre);
        }
    }

    return cant_entradas;
}

// Funcion que cambia los permisos de un fichero/directorio
int mi_chmod(const char *camino, unsigned char permisos){
    struct superbloque SB;
    bread(posSB, &SB);
    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;

    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    mi_chmod_f(p_inodo, permisos);
    return EXITO;
}

// Funcion que obtiene los stats de un fichero/directorio
int mi_stat(const char *camino, struct STAT *p_stat){
    struct superbloque SB;
    bread(posSB, &SB);
    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    int error;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0){ // solo es necesario otorgar permisos de lectura
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    
    mi_stat_f(p_inodo, p_stat);
    return p_inodo;
}

// escribir contenido en un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;

    #if (USARCACHE > 0) // 1. Buscar en la caché
        #if (USARCACHE == 1)
            if (strcmp(camino, UltimaEntradaEscritura.camino) == 0) {
                p_inodo = UltimaEntradaEscritura.p_inodo;
                #if (DEBUG && NIVEL9)
                    printf(GRAY "[mi_write() → Utilizamos la caché de escritura]\n" RESET);
                #endif
            }
        #else     // Búsqueda en tabla (FIFO o LRU)
            for (int i = 0; i < CACHE_SIZE; i++) {
                if (strcmp(camino, UltimasEntradas[i].camino) == 0) {
                    p_inodo = UltimasEntradas[i].p_inodo;
                    #if (USARCACHE == 3)
                        gettimeofday(&UltimasEntradas[i].ultima_consulta, NULL); // Actualizar sello LRU
                    #endif
                    #if (DEBUG && NIVEL9)
                        printf(GRAY "[mi_write() → Utilizamos cache[%d]: %s]\n" RESET, i, camino);
                    #endif
                    break;
                }
            }
        #endif
    #endif

    // 2. Si no estaba en caché (p_inodo sigue siendo 0), buscar y actualizar
    if (p_inodo == 0) {
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0) {
            return error;
        }

    #if (USARCACHE == 1)
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
        #if (DEBUG && NIVEL9)
            printf(GRAY "[mi_write() → Actualizamos la caché de escritura]\n" RESET);
        #endif
    #elif (USARCACHE == 2) // FIFO
        int pos = ultima_posicion_insertada % CACHE_SIZE;
        strcpy(UltimasEntradas[pos].camino, camino);
        UltimasEntradas[pos].p_inodo = p_inodo;
        #if (DEBUG && NIVEL9)
            printf(GRAY "[mi_write() → Reemplazamos cache[%d]: %s (FIFO)]\n" RESET, pos, camino);
        #endif
        ultima_posicion_insertada++;
    #elif (USARCACHE == 3) // LRU
        int pos_lru = 0;
        for (int i = 1; i < CACHE_SIZE; i++) {
            // Buscamos el que tenga el tiempo más antiguo
            if (UltimasEntradas[i].ultima_consulta.tv_sec < UltimasEntradas[pos_lru].ultima_consulta.tv_sec ||
               (UltimasEntradas[i].ultima_consulta.tv_sec == UltimasEntradas[pos_lru].ultima_consulta.tv_sec &&
                UltimasEntradas[i].ultima_consulta.tv_usec < UltimasEntradas[pos_lru].ultima_consulta.tv_usec)) {
                pos_lru = i;
            }
        }
        strcpy(UltimasEntradas[pos_lru].camino, camino);
        UltimasEntradas[pos_lru].p_inodo = p_inodo;
        gettimeofday(&UltimasEntradas[pos_lru].ultima_consulta, NULL);
        #if (DEBUG && NIVEL9)
            printf(GRAY "[mi_write() → Reemplazamos cache[%d]: %s (LRU)]\n" RESET, pos_lru, camino);
        #endif
    #endif
    }

    mi_waitSem(); // Nivel 11 
    int returnValue = mi_write_f(p_inodo, buf, offset, nbytes);
    mi_signalSem();

    return returnValue;
}

// leer contenido de un fichero
// leer contenido de un fichero con soporte de caché FIFO/LRU
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    int p_inodo = -1;
    unsigned int p_inodo_dir = 0, p_entrada = 0;
    int error;

    #if (USARCACHE > 0)
        // 1. Buscar en la caché 
        #if (USARCACHE == 1)
            if (strcmp(camino, UltimaEntradaLectura.camino) == 0) {
                p_inodo = UltimaEntradaLectura.p_inodo;
                #if (DEBUG && NIVEL9)
                    printf(GRAY "\n[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
                #endif
            }
        #else 
            // Búsqueda en tabla (FIFO o LRU)
            for (int i = 0; i < CACHE_SIZE; i++) {
                if (strcmp(camino, UltimasEntradas[i].camino) == 0) {
                    p_inodo = UltimasEntradas[i].p_inodo;
                    #if (USARCACHE == 3)
                        gettimeofday(&UltimasEntradas[i].ultima_consulta, NULL); // Actualizar sello LRU
                    #endif
                    #if (DEBUG && NIVEL9)
                        printf(GRAY "[mi_read() → Utilizamos cache[%d]: %s]\n" RESET, i, camino);
                    #endif
                    break;
                }
            }
        #endif
    #endif

    // 2. Si no estaba en caché, buscar y actualizar la tabla
    if (p_inodo == -1) {
        unsigned int p_inodo_aux;
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo_aux, &p_entrada, 0, 0)) < 0) {
            return error;
        }
        p_inodo = p_inodo_aux;

        #if (USARCACHE == 1)
            strcpy(UltimaEntradaLectura.camino, camino);
            UltimaEntradaLectura.p_inodo = p_inodo;
            #if (DEBUG && NIVEL9)
                printf(GRAY "[mi_read() → Actualizamos la caché de lectura]\n" RESET);
            #endif
        #elif (USARCACHE == 2) // FIFO
            int pos = ultima_posicion_insertada % CACHE_SIZE; // Uso circular del array
            strcpy(UltimasEntradas[pos].camino, camino);
            UltimasEntradas[pos].p_inodo = p_inodo;
            #if (DEBUG && NIVEL9)
                printf(GRAY "[mi_read() → Reemplazamos cache[%d]: %s (FIFO)]\n" RESET, pos, camino);
            #endif
            ultima_posicion_insertada++;
        #elif (USARCACHE == 3) // LRU
            int pos_lru = 0;
            for (int i = 1; i < CACHE_SIZE; i++) {
                // Buscamos la entrada con el sello de tiempo más antiguo
                if (UltimasEntradas[i].ultima_consulta.tv_sec < UltimasEntradas[pos_lru].ultima_consulta.tv_sec ||
                   (UltimasEntradas[i].ultima_consulta.tv_sec == UltimasEntradas[pos_lru].ultima_consulta.tv_sec &&
                    UltimasEntradas[i].ultima_consulta.tv_usec < UltimasEntradas[pos_lru].ultima_consulta.tv_usec)) {
                    pos_lru = i;
                }
            }
            strcpy(UltimasEntradas[pos_lru].camino, camino);
            UltimasEntradas[pos_lru].p_inodo = p_inodo;
            gettimeofday(&UltimasEntradas[pos_lru].ultima_consulta, NULL); // Sello en microsegundos[cite: 1]
            #if (DEBUG && NIVEL9)
                printf(GRAY "[mi_read() → Reemplazamos cache[%d]: %s (LRU)]\n" RESET, pos_lru, camino);
            #endif
        #endif
    }

    mi_waitSem(); // Nivel 11 
    int returnvalue = mi_read_f(p_inodo, buf, offset, nbytes);
    mi_signalSem();

    return returnvalue;
}

//Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1 
int mi_link(const char *camino1, const char *camino2){
    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;
    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    int error;
    struct entrada entrada;
    struct inodo inodo;

    mi_waitSem();

    // camino1 debe existir
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0)) < 0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // leer inodo original
    if (leer_inodo(p_inodo1, &inodo) < 0){
        return FALLO;
    }

    // no permitir enlaces a directorios
    if (inodo.tipo != 'f'){
        fprintf(stderr, RED "Error: No se permiten enlaces a directorios.\n" RESET);
        return FALLO;
    }

    // comprobar permisos lectura
    if (!(inodo.permisos & 4)){
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        return FALLO;
    }

    // crear entrada camino2
    // debe NO existir
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // leer entrada creada
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) < 0){
        liberar_inodo(p_inodo2);
        return FALLO;
    }

    // hacer que apunte al mismo inodo
    entrada.ninodo = p_inodo1;

    // escribir entrada modificada
    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) < 0){
        liberar_inodo(p_inodo2);
        return FALLO;
    }

    // liberar el inodo reservado para camino2
    if (liberar_inodo(p_inodo2) < 0){
        return FALLO;
    }

    // actualizar nlinks y ctime
    inodo.nlinks++;
    inodo.ctime = time(NULL);

    if (escribir_inodo(p_inodo1, &inodo) < 0){
        return FALLO;
    }
    mi_signalSem();
    return EXITO;
}

int mi_unlink(const char *camino){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    struct entrada ultima_entrada;
    struct inodo inodo, inodo_dir;
    int error;

    mi_waitSem();

    if (strcmp(camino, "/") == 0){
        return FALLO;
    }

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    if (leer_inodo(p_inodo, &inodo) < 0){
        return FALLO;
    }

    // si es directorio debe estar vacío
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0){
        fprintf(stderr, RED "Error: El directorio %s no está vacío\n" RESET, camino);
        return FALLO;
    }

    if (leer_inodo(p_inodo_dir, &inodo_dir) < 0){
        return FALLO;
    }

    int num_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    // si no es la última entrada
    if (p_entrada != num_entradas - 1){
        if (mi_read_f(p_inodo_dir, &ultima_entrada, (num_entradas - 1) * sizeof(struct entrada), sizeof(struct entrada)) < 0){
            return FALLO;
        }

        if (mi_write_f(p_inodo_dir, &ultima_entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0){
            return FALLO;
        }
    }

    // truncar una entrada
    if (mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada)) < 0){
        return FALLO;
    }

    // actualizar links
    inodo.nlinks--;

    if (inodo.nlinks == 0){
          
        liberar_inodo(p_inodo);
        
    } else {
        inodo.ctime = time(NULL);
        escribir_inodo(p_inodo, &inodo);
    }
    mi_signalSem();
    return EXITO;
}