#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "extra.h"
#include "errores.h"
#include "logistica.h"
#define QLENGTH 5

char *nombre = NULL;    /*Nombre de la bomba*/
int gas      = 0;       /*Cantidad de gasolina actual*/
int max      = 0;       /*Cantidad maxima de gasolina*/
int entrada  = 0;       /*Cantidad de gasolina que entra en litros por minuto*/
int resp     = 0;       /*Tiempo de respuesta del centro*/
int tiempo   = 0;       /*Tiempo transcurrido desde que comenzo a operar el centro*/
int ticket   = 0;       /*Numerador de tickets*/
sem_t sem;              /*Semaforo para control de acceso a 'gas'*/
sem_t semf;             /*Semaforo para control de escritura en el log*/
FILE *out;              /*Arhcivo del log*/

void generar_random(char *string) {

     int i;

     for (i = 0; i < 128; ++i)
     {
         string[i] = 'a' + i;
     }
}

char *generar_respuesta(char *string) {

    char *resp;

    //exec

    return resp;
}

void actualizar_ticket(int ticket, unsigned long ip) {

}

int autenticado(int ticket, unsigned long ip) {

    return 0;
}

//La cola debe tener IP del cliente, ticket, hora de ultima conexion, respuesta de desafio enviado y estado de autenticacion
int *pedir_gasolina_1(argp, rqstp)
    int *argp;
    CLIENT *rqstp;
{

    static int result;
    unsigned long ip;
    int ticket = *argp;

    //obtenemos el nombre o el IP de la bomba
    // ip = rqstp->rq_xprt->xp_raddr.sin_addr.s_addr;

    result = autenticado(ticket, ip);

    /*Verifica que este auteticado*/
    if (result == -1) {

        return &result;
    /*Si esta en proceso de una autenticacion*/
    } else if (result == -2) {

        return &result;
    }

    //Que pasa si ya se esta en el medio de una atenticacion para ese servidor
    //deberiamos tener una respuesta que sea "autenticacion en proceso"

    actualizar_ticket(ticket, ip);

    result = 0;
    sem_wait(&sem);

    /*Si hay suficiente gasolina para enviar*/
    if (gas >= CARGA) {

        gas -= CARGA;
        result = 1;

        if (gas == 0) {

            /*Escritura en log*/
            sem_wait(&semf);
            fprintf(out, "Tanque vacio: %d\n", tiempo);
            printf("Tanque vacio: %d\n", tiempo);
            sem_post(&semf);
        }
    }
    sem_post(&sem);

    return &result;
}

char **pedir_desafio_1(argp, rqstp)
    void *argp;
    CLIENT *rqstp;
{

    static char *result;
    char *resp;

    result = (char *) malloc(128 * sizeof(char));

    generar_random(result);
    resp = generar_respuesta(result);

    ++ticket;
    sprintf(result, "%d&%s", ticket, result);

    return &result;
}

int *autenticar_1(argp, rqstp)
    char **argp;
    CLIENT *rqstp;
{

    static int result;

    return &result;
}

int *pedir_tiempo_1(argp, rqstp)
    void *argp;
    CLIENT *rqstp;
{

    static int result;

    return &result;
}

/*
 * Funcion de hilo, aumenta el inventario de gasolina del centro
 */
void *control_gas(){

    /*Ciclo infinito que recibe gasolina*/
    while (tiempo < LIMITE){

        ++tiempo;
        /*Duerme por un minuto (0.1 seg)*/
        usleep(MINUTO);

        /*wait para accesar a 'gas'*/
        sem_wait(&sem);

        /*Si hay espacio suficiente para la entrega llegando*/
        if (entrada + gas < max){

            gas += entrada;
        /*Si se acaba de llenar el tanque*/
        } else if (gas < max) {

            gas = max;

            /*Escritura en log*/
            sem_wait(&semf);
            fprintf(out, "Tanque full: %d\n", tiempo);
            printf("Tanque full: %d\n", tiempo);
            sem_post(&semf);
        }

        /*signal para liberar 'gas'*/
        sem_post(&sem);
    }

    /*Termina el hilo*/
    exit(0);
}

int principal(int argc, char **argv) {

    char flog[50] = "log_";             /*Nombre del archivo de log*/


    /*Inicializacion semaforo en 1*/
    sem_init(&sem,0,1);
    sem_init(&semf,0,1);

    /*procedimiento que obtiene los valores generales*/
    if (llamadaC(argc, argv, &nombre, &max, &gas, &resp, &entrada) < 0) {
        return -1;
    }

    /*Arma el nombre del archivo de log*/
    strcat(flog, nombre);
    strcat(flog, ".txt");
    /*Abre el archivo de log*/
    if ((out = fopen(flog, "w")) == NULL) {

        return errorFile(__LINE__);
    }

    fprintf(out, "Estado inicial: %d\n", gas);
    printf("Estado inicial: %d\n", gas);

    /*Si comenzo vacio el tanque*/
    if (gas == 0) {

        fprintf(out, "Tanque vacio: %d\n", tiempo);
        printf("Tanque vacio: %d\n", tiempo);
    }

    /*Si comenzo lleno el tanque*/
    if (gas == max) {

        fprintf(out, "Tanque full: %d\n", tiempo);
        printf("Tanque full: %d\n", tiempo);
    }

    /*Control de gasolina y termina cuando pase el tiempo*/
    control_gas();

    fclose(out);

    exit(0);
}