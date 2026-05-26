//Antonio García Font y Maria Isabel Herrero Soteras  

# Sistemas-Operativos-2 - Entrega 3
Proyecto Sistemas Operativos 2 - Ingenieria Informatica UIB

## ejecutar scripte1.sh
Si se quiere ejecutar el script1, se tendra que activar todos los debug de ficheros_basico.c a 1 para
que salga igual que en el ejemplo script1 de la evaluacion.

## Que esta Dentro de la entrega?
Hasta el nivel 13
Mejora de mi_ls -l
Cache de directorios Normal, FIFO, y LRU
mi_touch y mi_rmdir

## Cache de caminos
Para cambiar entre FIFO LRU, o el basico Ultima L/E, ir a directorios.h y cambiar USARCACHE, ahi se comenta los modos.
Para probar L/E:
./mi_mkfs disco 100000
./mi_touch disco 6 /fic3
./mi_escribir_varios disco /fic3 "--texto repetido en 10 bloques--" 0 
habra que activar el debug 9 en directorios.c para ver extra info

## Solucionado de entrega2
Ahora mi_dir y buscar_entrada no leen entrada a entrada, sino que van leyendo en un buffer de tamaño blocksize hasta que lo encuentren o hasta
que no haya mas entradas en el inodo.

## Funciones

### liberar_bloques_inodo 
Introducida V2.

#### liberar_indirectos_recursivo
En esta funcion hemos añadido dos parametros para debugear, bwrite_p y bread_p, para poder imprimir los debugs de bwrite y bread sin problema.
Hemos tenido problemas a la hora de ver como sacar esta informacion y fuimos con esta solucion.

Tambien nos sucede, cuando va a guardar un bloque, primero saltara, y luego guardara, solo da problema a la hora de ver debugs, ya que estaran girados, 
esto se debe a que solo guardamos al final de la iteracion del bloque de punteros para ahorrar bwrites, y que no vaya guardando todo el rato, 
pero por lo tanto, nos hara saltos antes del mismo save, hemos mirado maneras de cambiar el debug, pero no encontramos una 
solucion que nos gustara, asi que decidimos dejarlo asi, ya que la informacion aunque algo desorganizada, es correcta.