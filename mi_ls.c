#include "directorios.h"

int main(int argc, char **argv){
    
    char buffer[TAMBUFFER];
    char *ruta;
    char flag = 0;  // 0 = simple, 'l' = extendido
    int total = 0;
    
    // Comprobamos la sintaxis
    if(argc < 3 || argc > 4){
        fprintf(stderr, RED "Sintaxis: ./mi_ls <disco> </ruta>  o  ./mi_ls -l <disco> </ruta>\n" RESET);
        return FALLO;
    }
    
    // Parsear argumentos
    if(argc == 4){
        // Formato: ./mi_ls -l disco /ruta
        if(strcmp(argv[1], "-l") != 0){
            fprintf(stderr, RED "Error: opción no válida. Use -l para listado extendido.\n" RESET);
            return FALLO;
        }
        flag = 'l';
        ruta = argv[3];
        
        if(bmount(argv[2]) == FALLO){
            return FALLO;
        }
    } else {
        // Formato: ./mi_ls disco /ruta
        ruta = argv[2];
        
        if(bmount(argv[1]) == FALLO){
            return FALLO;
        }
    }
    
    // Llamar a mi_dir() según el formato
    buffer[0] = '\0';
    total = mi_dir(ruta, buffer, 'd', flag);  // 'd' porque siempre listamos directorios
    
    if(total < 0){
        mostrar_error_buscar_entrada(total);
        bumount();
        return FALLO;
    }
    
    // Mostrar resultados
    if(flag == 'l'){
        // CAMBIO: Solo imprimimos el Total si la ruta termina en / (es un directorio)
        // Si es un fichero individual (/ruta/fichero), el PDF muestra que NO sale el total.
        if (ruta[strlen(ruta) - 1] == '/') {
            printf("Total: %d\n", total);
        }
        
        // Imprimimos el buffer (que ya contendrá la cabecera y los datos)
        printf("%s", buffer);
    } else {
        // En listado simple, imprimimos el total y los nombres en una línea
        printf("Total: %d\n", total);

        char *token = strtok(buffer, "\t");
        while(token != NULL){
            printf("%s  ", token);
            token = strtok(NULL, "\t");
        }
        printf("\n");
    }
    
    bumount();
    return EXITO;
}