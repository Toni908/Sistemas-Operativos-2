//Antonio García Font y Maria Isabel Herrero Soteras
#include "verificacion.h"

#define NIVEL13 1

int main(int argc, char *argv[]) {
    // 1. Comprobar la sintaxis
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <directorio_simulación>\n", argv[0]);
        return ERROR;
    }

    // 2. Montar el dispositivo virtual
    if (bmount(argv[1]) == ERROR) {
        return ERROR;
    }

    // 3. Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    struct STAT stat;
    if (mi_stat(argv[2], &stat) < 0) {
        bumount();
        return ERROR;
    }

    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);
    
    // 4. Si numentradas != NUMPROCESOS entonces ERROR
    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, "Error: El número de entradas no coincide con el número de procesos.\n");
        bumount();
        return ERROR;
    }

    printf("dir_sim: %s\n", argv[2]);
    printf("numentradas: %d NUMPROCESOS: %d\n", numentradas, NUMPROCESOS);

    // 5. Crear el fichero "informe.txt" dentro del directorio de simulación
    char camino_informe[128];
    sprintf(camino_informe, "%sinforme.txt", argv[2]);
    if (mi_creat(camino_informe, 6) < 0) {
        bumount();
        return ERROR;
    }

    // 6. Leer los directorios correspondientes a los procesos
    struct entrada entradas_procesos[NUMPROCESOS];
    if (mi_read(argv[2], entradas_procesos, 0, sizeof(entradas_procesos)) < 0) {
        bumount();
        return ERROR;
    }

    int offset_informe = 0; // Para llevar la cuenta de dónde escribir en informe.txt

    // 7. Para cada entrada de directorio de un proceso hacer:
    for (int i = 0; i < NUMPROCESOS; i++) {
        // Extraer el PID a partir del nombre de la entrada y guardarlo
        pid_t pid = atoi(strchr(entradas_procesos[i].nombre, '_') + 1);
        
        struct INFORMACION info;
        info.pid = pid;
        info.nEscrituras = 0;

        char camino_prueba[128];
        sprintf(camino_prueba, "%s%s/prueba.dat", argv[2], entradas_procesos[i].nombre);

        int cant_registros_buffer_escrituras = 256;
        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        
        int offset_lectura = 0;
        int bytes_leidos = 0;

        // Mientras haya escrituras en prueba.dat hacer:
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        while ((bytes_leidos = mi_read(camino_prueba, buffer_escrituras, offset_lectura, sizeof(buffer_escrituras))) > 0) {
            
            int registros_leidos = bytes_leidos / sizeof(struct REGISTRO);
            
            for (int j = 0; j < registros_leidos; j++) {
                // Si la escritura es válida (coincide el PID)
                if (buffer_escrituras[j].pid == info.pid) {
                    if (info.nEscrituras == 0) {
                        // Inicializar con la primera escritura validada encontrada
                        info.MenorPosicion = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                    } else {
                        // Comparar e ir actualizando los structs
                        if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura) {
                            info.PrimeraEscritura = buffer_escrituras[j];
                        }
                        if (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura) {
                            info.UltimaEscritura = buffer_escrituras[j];
                        }
                        if (buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro) {
                            info.MenorPosicion = buffer_escrituras[j];
                        }
                        if (buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro) {
                            info.MayorPosicion = buffer_escrituras[j];
                        }
                    }
                    info.nEscrituras++;
                }
            }
            offset_lectura += bytes_leidos;
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        }

        #if (NIVEL13)
            printf("[%d) %d escrituras validadas en %s]\n", i + 1, info.nEscrituras, camino_prueba);
        #endif

        // Añadir la información del struct info al fichero informe.txt
        char buffer_cadena[2048];
        char primera[64], ultima[64], menor[64], mayor[64];
        struct tm *tm_info;

        // Formateamos las fechas utilizando strftime
        tm_info = localtime(&info.PrimeraEscritura.fecha);
        strftime(primera, sizeof(primera), "%a %b %d %H:%M:%S %Y", tm_info);

        tm_info = localtime(&info.UltimaEscritura.fecha);
        strftime(ultima, sizeof(ultima), "%a %b %d %H:%M:%S %Y", tm_info);

        tm_info = localtime(&info.MenorPosicion.fecha);
        strftime(menor, sizeof(menor), "%a %b %d %H:%M:%S %Y", tm_info);

        tm_info = localtime(&info.MayorPosicion.fecha);
        strftime(mayor, sizeof(mayor), "%a %b %d %H:%M:%S %Y", tm_info);

        sprintf(buffer_cadena,
                "PID: %d\n"
                "Numero de escrituras: %d\n"
                "Primera Escritura\t%d\t%d\t%s\n"
                "Ultima Escritura\t%d\t%d\t%s\n"
                "Menor Posición\t\t%d\t%d\t%s\n"
                "Mayor Posición\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, primera,
                info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, ultima,
                info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, menor,
                info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, mayor);

        // Escribimos en el informe y actualizamos el offset
        mi_write(camino_informe, buffer_cadena, offset_informe, strlen(buffer_cadena));
        offset_informe += strlen(buffer_cadena);
    }

    // 8. Desmontar el dispositivo virtual
    bumount();

    return 0;
}