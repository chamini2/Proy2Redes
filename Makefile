FLAGS = -std=c99 -pedantic -Wall -Wextra -Wwrite-strings

all: cleanC rpc bomba centro md5

#bomba (cliente)
bomba: bomba.o errores.o extra.o queue.o logistica_clnt.o
	-gcc $(FLAGS) -pthread logistica_clnt.o bomba.o errores.o extra.o queue.o -o bomba
	echo ""

bomba.o: bomba.c errores.h extra.h queue.h
	gcc -c  $(FLAGS) -pthread bomba.c

#centro (servidor)
centro: centro.o errores.o extra.o logistica_svc.o
	-gcc $(FLAGS) -pthread logistica_svc.o centro.o errores.o extra.o -o centro
	echo ""

centro.o: centro.c errores.h extra.h queue.h
	gcc -c $(FLAGS) -pthread centro.c

#procedimientos extra
extra.o: extra.c extra.h errores.h
	gcc -c $(FLAGS) extra.c

#manejo de errores
errores.o: errores.c errores.h
	gcc -c $(FLAGS) errores.c

#cola
queue.o: queue.c queue.h errores.h
	gcc -c $(FLAGS) queue.c

#RPC
rpc: logistica.x
	rpcgen logistica.x

logistica_svc.o: logistica_svc.c
	gcc -c $(FLAGS) logistica_svc.c

logistica_clnt.o: logistica_clnt.c
	gcc -c $(FLAGS) logistica_clnt.c

#md5
md5:
	cd md5-c/; make

cleanC:
	-rm *.o bomba centro
	-rm log_*.txt
	clear

clean: cleanC
	--cd md5-c/; make clean
	clear;