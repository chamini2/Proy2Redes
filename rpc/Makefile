FLAGS = -pedantic -std=c99 -Wall -Wextra -Wwrite-strings

all: cleanC centro bomba md5

#bomba (cliente)
bomba: bomba.o list.o errores.o extra.o queue.o logistica_clnt.c
	-gcc $(FLAGS) -pthread -lrpcsvc bomba.o logistica_clnt.c list.o errores.o extra.o queue.o -o bomba
	echo ""

bomba.o: bomba.c errores.h extra.h queue.h list.h
	gcc -c  $(FLAGS) -pthread -lrpcsvc bomba.c logistica_clnt.c

#centro (servidor)
centro: centro.o list.o errores.o extra.o logistica_svc.c
	-gcc $(FLAGS) -pthread -lrpcsvc logistica_svc.c centro.o errores.o extra.o list.o -o centro
	echo ""

centro.o: centro.c errores.h extra.h queue.h list.h
	gcc -c $(FLAGS) -pthread -lrpcsvc centro.c logistica_svc.c

#procedimientos extra
extra.o: extra.c extra.h errores.h
	gcc -c $(FLAGS) extra.c

#manejo de errores
errores.o: errores.c errores.h
	gcc -c $(FLAGS) errores.c

#cola
queue.o: queue.c queue.h errores.h
	gcc -c $(FLAGS) queue.c

list.o: list.c list.h
	gcc -c $(FLAGS) list.c 

#RPC
rpc: logistica.x
	rpcgen logistica.x

# logistica_svc.o: logistica_svc.c
# 	gcc -c $(FLAGS) logistica_svc.c

# logistica_clnt.o: logistica_clnt.c
# 	gcc -c $(FLAGS) logistica_clnt.c

#md5
md5:
	cd md5-c/; make

cleanC:
	-rm *.o bomba centro
	-rm log_*.txt
	clear
	clear

clean: cleanC
	--cd md5-c/; make clean
	clear
