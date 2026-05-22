//Antonio García Font y Maria Isabel Herrero Soteras  
#include "simulacion.h"

int acabados = 0;

void reaper(int signum) {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: ./simulacion <nombre_disco>\n");
        return -1;
    }

    signal(SIGCHLD, reaper);
    
    // Crear directorio de simulación con fecha/hora [cite: 23]
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dir_sim[64];
    sprintf(dir_sim, "/simul_%04d%02d%02d%02d%02d%02d/", 
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (bmount(argv[1]) == -1) return -1;
    mi_mkdir(dir_sim);
    bumount();

    for (int i = 1; i <= NUMPROCESOS; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Hijo
            bmount(argv[1]);
            
            char path_proceso[128];
            sprintf(path_proceso, "%sproceso_%d/", dir_sim, getpid());
            mi_mkdir(path_proceso);
            
            char path_fichero[256];
            sprintf(path_fichero, "%sprueba.dat", path_proceso);
            mi_creat(path_fichero, 6); // Asumiendo mi_creat o similar

            srand(time(NULL) + getpid());

            for (int n = 1; n <= NUMESCRITURAS; n++) {
                struct REGISTRO reg;
                reg.fecha = time(NULL);
                reg.pid = getpid();
                reg.nEscritura = n;
                reg.nRegistro = rand() % REGMAX;

                mi_write(path_fichero, &reg, reg.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                usleep(50000); // 0.05 seg [cite: 57]
            }
            
            bumount();
            exit(0);
        } else if (pid > 0) {
            usleep(150000); // 0.15 seg entre procesos [cite: 28]
        }
    }

    while (acabados < NUMPROCESOS) {
        pause();
    }

    printf("Simulación finalizada.\n");
    return 0;
}