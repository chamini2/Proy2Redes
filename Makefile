all:
	rpcgen logistica.x

clean:
	rm *.o bomba centro logistica_clnt.c logistica_svc.c logistica.h