# Sistemas-Operativos-2 - Entrega 1
Proyecto Sistemas Operativos 2 - Ingenieria Informatica UIB

## Funciones

### liberar_indirectos_recursivo
En esta funcion hemos añadido dos parametros para debugear, bwrite_p y bread_p, para poder imprimir los debugs de bwrite y bread sin problema.
Hemos tenido problemas a la hora de ver como sacar esta informacion y fuimos con esta solucion.

Tambien nos sucede, cuando va a guardar un bloque, primero saltara, y luego guardara, solo da problema a la hora de ver debugs, ya que estaran girados, esto se debe a que solo guardamos al final de la iteracion
del bloque de punteros para ahorrar bwrites, y que no vaya guardando todo el rato, pero por lo tanto, nos hara saltos antes del mismo save, hemos mirado maneras de cambiar el debug, pero no encontramos una
solucion que nos gustara, asi que decidimos dejarlo asi, ya que la informacion aunque algo desorganizada, es correcta.



# Sistemas-Operativos-2 - Entrega 2

## Funciones

### Funciones opcionales

#### Cache de caminos
Caches de nivel 9 FIFO Y LRU implementados, adjuntamos la prueba del nivel 9 que trabajan estos mismos para que se puedan probar, llamado prueba_cache_tabla.c, para ejecutar la prueba:
$ ./mi_mkfs disco 100000
$ ./prueba_cache_tabla disco ”hola”
Para cambiar entre FIFO LRU, o el basico Ultima L/E, ir a directorios.h y cambiar USARCACHE, ahi se comenta los modos.

#### No implementado
No estan implementadas las funciones mi_rmdir ni mi_rm_r, el archivo scripte2.sh a sido modificado, mas concretamente los dos ultimos mi_rmdir han sido modificados por mi_rm, estan aseñalados, para que de el mismo
resultado los archivos, un mi_rmdir no fue quitado ya que no molesta la ejecucion de los archivos.

## Scripte2.sh modificado
Comentado en funciones opcionales, dos lineas fueron modificadas del scripte2.sh ya que no utilizamos mi_rmdir para conseguir el mismo resultado que en las pruebas con mi_rm.

