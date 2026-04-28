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

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {

    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;

    char inicial[TAMNOMBRE];
    char final[strlen(camino_parcial)];
    char tipo;

    int cant_entradas_inodo;
    int num_entrada_inodo = 0;

    if(bread(posSB, &SB) == FALLO) return FALLO;

    // Caso raíz
    if (strcmp(camino_parcial, "/") == 0) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0; // no se usara, valor dummy
        return EXITO;
    }

    // Extraer camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0) return ERROR_CAMINO_INCORRECTO;

    #if DEBUG
        printf(GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
    #endif

    // Leer inodo del directorio
    if (leer_inodo(*p_inodo_dir, &inodo_dir) < 0) return ERROR_PERMISO_LECTURA;


    // Comprobar permisos de lectura
    if (!(inodo_dir.permisos & 4)) {
        #if DEBUG
            printf(GRAY "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }
    
    // Calcular número de entradas, pa saber hasta donde iterar
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    // Buscar entrada
    while (num_entrada_inodo < cant_entradas_inodo) {
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) return FALLO;
        if (strcmp(inicial, entrada.nombre) == 0) break;
        num_entrada_inodo++;
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
            entrada.ninodo = reservar_inodo('d', permisos);

            #if DEBUG
                printf(GRAY "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n" RESET, entrada.ninodo, permisos, inicial);
            #endif
        } else {
            entrada.ninodo = reservar_inodo('f', permisos);
            #if DEBUG
                printf(GRAY "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n" RESET, entrada.ninodo, permisos, inicial);
            #endif
        }

        if (entrada.ninodo < 0) return FALLO;

        // Escribir entrada al final
        if (mi_write_f(*p_inodo_dir, &entrada, cant_entradas_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            liberar_inodo(entrada.ninodo);
            return FALLO;
        }

        #if DEBUG
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
    unsigned int p_inodo_dir = 0; // hay que meter una variable y pasarla como puntero no sirve solo con 0
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    return buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
}

//La funcion pone el contenido del directorio/fichero en un buffer de memoria
int mi_dir(const char *camino, char *buffer, char tipo, char flag){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    struct inodo inodo;
    struct entrada entrada;
    
    int error;
    buffer[0] = '\0';
    
    // Buscar entrada
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0){
        return error;
    }
    
    // Leer inodo
    if(leer_inodo(p_inodo, &inodo) < 0){
        return FALLO;
    }
    
    // Comprobar permisos de lectura
    if(!(inodo.permisos & 4)){
        return ERROR_PERMISO_LECTURA;
    }
    
    // Si es un fichero (mejora opcional)
    if(inodo.tipo == 'f' && flag == 'l'){
        struct tm *tm_info;
        char tmp[TAMFILA];

        // Añadir cabecera al principio del buffer
        strcat(buffer, "Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
        strcat(buffer, "----------------------------------------------------------------\n");
        
        sprintf(tmp, "%c\t", inodo.tipo);
        strcat(buffer, tmp);
        
        // permisos
        strcat(buffer, (inodo.permisos & 4) ? "r" : "-");
        strcat(buffer, (inodo.permisos & 2) ? "w" : "-");
        strcat(buffer, (inodo.permisos & 1) ? "x\t" : "-\t");
        
        // fecha
        tm_info = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                tm_info->tm_year + 1900,
                tm_info->tm_mon + 1,
                tm_info->tm_mday,
                tm_info->tm_hour,
                tm_info->tm_min,
                tm_info->tm_sec);
        strcat(buffer, tmp);
        
        // tamaño
        sprintf(tmp, "%d\t", inodo.tamEnBytesLog);
        strcat(buffer, tmp);
        
        // nombre
        strcat(buffer, strrchr(camino, '/') + 1);
        strcat(buffer, "\n");
        
        return 1;
    }
    
    // Si es directorio
    int cant_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    struct inodo inodo_aux;
    
    if(flag == 'l' && cant_entradas > 0){
        strcat(buffer, "Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
        strcat(buffer, "----------------------------------------------------------------\n");
    }
    
    for(int i = 0; i < cant_entradas; i++){
        if(mi_read_f(p_inodo, &entrada, i * sizeof(struct entrada), sizeof(struct entrada)) < 0){
            return FALLO;
        }
        
        if(leer_inodo(entrada.ninodo, &inodo_aux) < 0){
            return FALLO;
        }
        
        // mjeora opcional tambien
        if(flag == 'l'){
            char tmp[TAMFILA];
            struct tm *tm_info;
            
            // tipo
            sprintf(tmp, "%c\t", inodo_aux.tipo);
            strcat(buffer, tmp);
            
            // permisos
            strcat(buffer, (inodo_aux.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo_aux.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo_aux.permisos & 1) ? "x\t" : "-\t");
            
            // fecha
            tm_info = localtime(&inodo_aux.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                    tm_info->tm_year + 1900,
                    tm_info->tm_mon + 1,
                    tm_info->tm_mday,
                    tm_info->tm_hour,
                    tm_info->tm_min,
                    tm_info->tm_sec);
            strcat(buffer, tmp);
            
            // tamaño
            sprintf(tmp, "%d\t", inodo_aux.tamEnBytesLog);
            strcat(buffer, tmp);
            
            // nombre
            strcat(buffer, entrada.nombre);
            strcat(buffer, "\n");
        } else {
            strcat(buffer, entrada.nombre);
            strcat(buffer, "\t");
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