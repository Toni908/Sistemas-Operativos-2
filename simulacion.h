//Antonio García Font y Maria Isabel Herrero Soteras  
#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>

#define REGMAX 500000
#define NUMPROCESOS 10
#define NUMESCRITURAS 20

struct REGISTRO {
    time_t fecha;
    pid_t pid;
    int nEscritura;
    int nRegistro;
};
