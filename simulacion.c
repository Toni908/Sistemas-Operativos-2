#include "simulacion.h"

// Implementación de la función enterrador
void reaper(int signum) {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char *argv[]) {
    // Comprobar sintaxis del comando
    if (argc != 2) {
        fprintf(stderr, "Sintaxis: ./simulacion <disco>\n");
        return EXIT_FAILURE;
    }

    // Asociar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    // Montar el dispositivo virtual (padre)
    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el dispositivo virtual\n");
        return EXIT_FAILURE;
    }

    // Crear el directorio de simulación con formato: /simul_aaaammddhhmmss/
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char dirname[100];
    sprintf(dirname, "/simul_%04d%02d%02d%02d%02d%02d/",
            tm_info->tm_year + 1900,
            tm_info->tm_mon + 1,
            tm_info->tm_mday,
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec);

    if (mi_creat(dirname, 6) == -1) {
        fprintf(stderr, "Error al crear el directorio de simulación\n");
        bumount();
        return EXIT_FAILURE;
    }

    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", 
           NUMPROCESOS, NUMESCRITURAS);

    // Generar NUMPROCESOS procesos
    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        pid_t pid = fork();

        if (pid == 0) {
            // PROCESO HIJO
            
            // Montar el dispositivo (cada hijo monta su propio descriptor)
            if (bmount(argv[1]) == -1) {
                fprintf(stderr, "[Hijo %d] Error al montar el dispositivo\n", getpid());
                exit(EXIT_FAILURE);
            }

            // Crear el directorio del proceso hijo: proceso_PID
            char process_dir[150];
            sprintf(process_dir, "%sproceso_%d/", dirname, getpid());
            
            if (mi_creat(process_dir, 6) == -1) {
                fprintf(stderr, "[Hijo %d] Error al crear directorio del proceso\n", getpid());
                bumount();
                exit(EXIT_FAILURE);
            }

            // Crear el fichero prueba.dat dentro del directorio del proceso
            char filepath[200];
            sprintf(filepath, "%sprueba.dat", process_dir);
            
            if (mi_creat(filepath, 6) == -1) {
                fprintf(stderr, "[Hijo %d] Error al crear fichero prueba.dat\n", getpid());
                bumount();
                exit(EXIT_FAILURE);
            }

            // Inicializar la semilla de números aleatorios
            srand(time(NULL) + getpid());

            // Realizar NUMESCRITURAS escrituras
            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) {
                // Inicializar el registro
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;  // [0, 499.999]

                // Escribir el registro en la posición aleatoria
                int offset = registro.nRegistro * sizeof(struct REGISTRO);
                
                if (mi_write(filepath, &registro, offset, sizeof(struct REGISTRO)) == -1) {
                    fprintf(stderr, "[Hijo %d] Error en escritura %d\n", getpid(), nescritura);
                    bumount();
                    exit(EXIT_FAILURE);
                }

                // Descomentar para debugging con pocos procesos
                // fprintf(stderr, "[simulación.c → Escritura %d en %sprueba.dat]\n", 
                //         nescritura, process_dir);

                // Esperar 0,05 segundos (50.000 microsegundos)
                usleep(50000);
            }

            // Mensaje final del proceso
            printf("[Proceso %d: Completadas %d escrituras en %sprueba.dat]\n", 
                   proceso, NUMESCRITURAS, process_dir);

            // Desmontar el dispositivo
            bumount();
            
            // Finalizar el proceso hijo
            exit(EXIT_SUCCESS);
            
        } else if (pid < 0) {
            fprintf(stderr, "Error al crear el proceso %d\n", proceso);
            bumount();
            return EXIT_FAILURE;
        }

        // PROCESO PADRE: Esperar 0,15 segundos antes de lanzar el siguiente proceso
        usleep(150000);  // 150.000 microsegundos = 0,15 segundos
    }

    // Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS) {
        pause();
    }

    // Desmontar el dispositivo (padre)
    bumount();

    return EXIT_SUCCESS;
}