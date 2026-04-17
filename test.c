#include <string.h>
#include <stdio.h>

#define NULL ((void *)0)

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    char copiaCamino[60];

    if (camino[0] != '/') {
        return -1;
    }

    strcpy(copiaCamino, camino);

    strcpy(inicial, strtok(copiaCamino + 1, "/"));

    char *resto = strtok(NULL, "/");

    if (camino[1 + strlen(inicial)] == '/') {
        strcpy(final, "/");
        if (resto != NULL) strcat(final, resto);
        *tipo = 'd';
        return 1;
    } else {
        if (resto != NULL) {
            strcpy(final, resto);
        } else {
            strcpy(final, "");  // ← ESTO ES LO IMPORTANTE
        }
        *tipo = 'f';
        return 0;
    }
}

int main(int argc, char **argv){
    char inicial[60];
    char final[60];
    char tipo;

    char camino[60] ;
    strcpy(camino, argv[1]);

    int i = extraer_camino(camino, inicial, final, &tipo);

    printf("%s, %s, %s, %c, %d", camino, inicial, final, tipo, i);

}