# Sistemas-Operativos-2 - Entrega 3
Proyecto Sistemas Operativos 2 - Ingenieria Informatica UIB

## ejecutar scripte1.sh
Si se quiere ejecutar el script1, se tendra que actibar el debug ENTREGA1 (para)
que salga igual que en el ejemplo script1 de la evaluacion, se encuentra en ficheros_basicos.c

## Que esta Dentro de la entrega?
Hasta el nivel 13
Mejora de mi_ls -l
Cache de directorios Normal, FIFO, y LRU
mi_touch y mi_rmdir

## Cache de caminos
Para cambiar entre FIFO LRU, o el basico Ultima L/E, ir a directorios.h y cambiar USARCACHE, ahi se comenta los modos.

## Solucionado de entrega2
Ahora mi_dir y buscar entrada no leen entrada a entrada, sino que van leyendo en un buffer hasta que lo encuentren o hasta
que no haya mas entradas en el inodo.