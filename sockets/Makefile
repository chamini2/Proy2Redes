all: clean bomba centro

#bomba (cliente)
bomba: bomba.o errores.o extra.o queue.o
	-gcc -pedantic -pthread bomba.o errores.o extra.o queue.o -o bomba
	echo ""

bomba.o: bomba.c errores.h extra.h queue.h
	gcc -c -g -pedantic -pthread bomba.c

#centro (servidor)
centro: centro.o errores.o extra.o queue.o
	-gcc -pedantic -pthread centro.o errores.o extra.o queue.o -o centro
	echo ""

centro.o: centro.c errores.h extra.h queue.h
	gcc -c -g -pedantic -pthread centro.c

#procedimientos extra
extra.o: extra.c extra.h errores.h
	gcc -c -g -pedantic extra.c

#manejo de errores
errores.o: errores.c errores.h
	gcc -c -g -pedantic errores.c

#cola
queue.o: queue.c queue.h errores.h
	gcc -c -g -pedantic queue.c

clean:
	-rm *.o bomba centro
	echo ""
	clear