//Antonio García Font y Maria Isabel Herrero Soteras  
#include "simulacion.h"

#define NIVEL12 1

int acabados = 0;

void reaper(int signum) {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(FALLO, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char *argv[]) {
    // 1. Comprobar sintaxis
    if (argc != 2) {
        fprintf(stderr, "Uso: ./simulacion <nombre_disco>\n");
        return FALLO;
    }

    // 2. Asociar señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);
    
    // 3. Montar el dispositivo (padre)
    if (bmount(argv[1]) == FALLO) return FALLO;

    // 4. Crear el directorio de simulación: /simul_aaaammddhhmmss/
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dir_sim[64];
    sprintf(dir_sim, "/simul_%04d%02d%02d%02d%02d%02d/", 
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    // Usamos mi_creat con permisos 6 (lectura + escritura) para crear el directorio
    mi_creat(dir_sim, 6); 
    bumount();

    // 5. Bucle de creación de procesos
    for (int i = 1; i <= NUMPROCESOS; i++) {
        pid_t pid = fork();
        
        if (pid == 0) { // Hijo
            // Montar el dispositivo (hijo)
            bmount(argv[1]);
            
            // Crear el directorio del proceso hijo añadiendo el PID
            char path_proceso[128];
            sprintf(path_proceso, "%sproceso_%d/", dir_sim, getpid());
            mi_creat(path_proceso, 6);
            
            // Crear el fichero prueba.dat
            char path_fichero[256];
            sprintf(path_fichero, "%sprueba.dat", path_proceso);
            mi_creat(path_fichero, 6);

            // Inicializar semilla
            srand(time(NULL) + getpid());

            // Bucle de escrituras
            for (int n = 1; n <= NUMESCRITURAS; n++) {
                struct REGISTRO reg;
                reg.fecha = time(NULL);
                reg.pid = getpid();
                reg.nEscritura = n;
                reg.nRegistro = rand() % REGMAX;

                // Escribir el registro
                mi_write(path_fichero, &reg, reg.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));

                #if (NIVEL12)
                    printf("[simulación.c → Escritura %d en %s]\n", n, path_fichero);
                #endif
                
                // Esperar 0,05 seg
                usleep(50000); 
            }
            
            bumount();
            exit(0); // Necesario para disparar SIGCHLD
            
        } else if (pid > 0) {
            // Esperar 0,15 seg para lanzar el siguiente proceso
            usleep(150000); 
        }
    }

    // 6. El padre espera a que todos los hijos terminen
    while (acabados < NUMPROCESOS) {
        pause();
    }

    printf("Simulación finalizada. Todos los procesos han terminado.\n");
    return EXITO;
}