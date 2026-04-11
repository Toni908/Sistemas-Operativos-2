# Sistemas-Operativos-2
Proyecto Sistemas Operativos 2 - Ingenieria Informatica UIB

## Funciones

### liberar_indirectos_recursivo
En esta funcion hemos añadido dos parametros para debugear, bwrite_p y bread_p, para poder imprimir los debugs de bwrite y bread sin problema.
Hemos tenido problemas a la hora de ver como sacar esta informacion y fuimos con esta solucion.

Tambien nos sucede, cuando va a guardar un bloque, primero saltara, y luego guardara, solo da problema a la hora de ver debugs, ya que estaran girados, esto se debe a que solo guardamos al final de la iteracion
del bloque de punteros para ahorrar bwrites, y que no vaya guardando todo el rato, pero por lo tanto, nos hara saltos antes del mismo save, hemos mirado maneras de cambiar el debug, pero no encontramos una
solucion que nos gustara, asi que decidimos dejarlo asi, ya que la informacion aunque algo desorganizada, es correcta.