#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "extra.h"
#include "errores.h"
#include "logistica.h"
#include "centro.h"
#define QLENGTH 5

char  *nombre  = NULL;    /*Nombre de la bomba*/
int   gas      = 0;       /*Cantidad de gasolina actual*/
int   max      = 0;       /*Cantidad maxima de gasolina*/
int   entrada  = 0;       /*Cantidad de gasolina que entra en litros por minuto*/
int   resp     = 0;       /*Tiempo de respuesta del centro*/
int   tiempo   = 0;       /*Tiempo transcurrido desde que comenzo a operar el centro*/
int   ticket   = 0;       /*Numerador de tickets*/
sem_t sem;                /*Semaforo para control de acceso a 'gas'*/
sem_t semf;               /*Semaforo para control de escritura en el log*/
sem_t seml;               /*Semaforo para la lista*/
FILE  *out;               /*Arhcivo del log*/
list  tickets = NULL;     /*Tickets de los clientes*/


int autenticado(pase *p) {

    iter it;
    pase *elem = NULL, *aux;

    sem_wait(&seml);
    it = list_iter(tickets);

    while (has_next_iter(it)) {
        aux = next_iter(&it);

        /*Si ya no esta autenticado*/
        if (aux->tiempo + 5 <= tiempo ) {

            remove_elem_list(&tickets, aux);
            continue;
        }

        /*Esta autenticado*/
        if (equals_pase(aux, p)) {

            elem = aux;
        }
    }
    sem_post(&seml);

    /*Si lo consigo && autenticado actualizo*/
    if (elem != NULL) {
        elem->tiempo = tiempo;
    }

    return -1;
}
int *
pedir_gasolina_1_svc(char **argp, struct svc_req *rqstp)
{

    static int result;
    char *nomB;
    char* aux = (char*) malloc(sizeof(char)*5);
    pase p;

    p.numero       = atoi(strtok(*argp, "&"));
    p.login        = nomB = strtok(NULL, "&");
    p.contrasena   = NULL;
    p.tiempo       = tiempo;
    p.autenticando = 0;

    /* Notar que es una variable local, solo se usa para chequeo,
     * no se almacena en ningun sitio
     */
    result = autenticado(&p);

    /*No esta autenticado*/
    if (result == -1) {

        /*Escritura en log*/
        sem_wait(&semf);
        fprintf(out, "Peticion: %d, %s, No autenticado\n", tiempo, nomB);
        printf("Peticion: %d, %s, No autenticado\n", tiempo, nomB);
        sem_post(&semf);

        return &result;
    /*Si esta en proceso de una autenticacion*/
    } else if (result == -2) {

        return &result;
    }

    /*Escritura en log*/
    sem_wait(&semf);
    fprintf(out, "Peticion: %d, %s, Autenticado\n", tiempo, nomB);
    printf("Peticion: %d, %s, Autenticado\n", tiempo, nomB);
    sem_post(&semf);

    result = 0;
    sem_wait(&sem);

    /*Si hay suficiente gasolina para enviar*/
    if (gas >= CARGA) {

        gas -= CARGA;
        result = 1;

        /*Escritura en log*/
        sem_wait(&semf);
        fprintf(out, "Suministro: %d, %s, Positiva, %d\n", tiempo, nomB, gas);
        printf("Suministro: %d, %s, Positiva, %d\n", tiempo, nomB, gas);

        if (gas == 0) {

            fprintf(out, "Tanque vacio: %d\n", tiempo);
            printf("Tanque vacio: %d\n", tiempo);
        }

        sem_post(&semf);

    } else {

        /*Escritura en log*/
        sem_wait(&semf);
        fprintf(out, "Suministro: %d, %s, Negativa, %d\n", tiempo, nomB, gas);
        printf("Suministro: %d, %s, Negativa, %d\n", tiempo, nomB, gas);
        sem_post(&semf);
    }


    sem_post(&sem);

    return &result;
}

char **
pedir_desafio_1_svc(char **argp, struct svc_req *rqstp)
{

    static char *result;
    char *resp;
    pase *p;

    if ((result = (char *) malloc(128 * sizeof(char))) == NULL) {

        errorMem(__LINE__);
        result = NULL;
        return &result;
    }

    if ((p = (pase *) malloc(sizeof(pase))) == NULL) {

        errorMem(__LINE__);
        free(result);
        result = NULL;
        return &result;
    }

    gen_random(result, 128);
    resp = md5(result);

    ++ticket;
    sprintf(result, "%d&%s", ticket, resp);

    p->numero       = ticket;
    p->login        = *argp;
    p->contrasena   = resp;
    p->tiempo       = tiempo;
    p->autenticando = 1;

    sem_wait(&seml);
    add_list(&tickets, p);
    sem_post(&seml);

    return &result;
}

void print_pase(pase *p) {

  printf("PASE");
  printf("tiempo: %d\n", p->tiempo);
  printf("login: %s\n", p->login);
  printf("numero: %d\n", p->numero);
}

int *
autenticar_1_svc(char **argp, struct svc_req *rqstp)
{

    static int result;
    char *nomB;
    pase *comp, *aux;

    if ((comp = (pase *) malloc(sizeof(pase))) == NULL) {

        errorMem(__LINE__);
        result = -1;
        return &result;
    }

    comp->numero       = atoi(strtok(*argp, "&"));
    comp->login        = nomB = strtok(NULL, "&");
    comp->contrasena   = strtok(NULL, "&");
    comp->tiempo       = tiempo;
    comp->autenticando = 0;


    sem_wait(&seml);
    printf("%s\n",tickets->first->info->login);
    aux = get_list(&tickets, comp);
    sem_post(&seml);
    if (aux == NULL){
      printf("aux es null\n");
      exit(1);
    }
    if (strcmp(comp->contrasena, aux->contrasena) == 0) {

        aux->autenticando = 0;
        aux->tiempo       = tiempo;

        result = 1;
    } else {

        result = -1;
    }

    return &result;
}

int *
pedir_tiempo_1_svc(void *argp, struct svc_req *rqstp)
{

    static int result;

    result = resp;

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

void *principal(void *arg) {
    struct args *flags = (struct args*) arg;
    int argc = flags->argc;
    char **argv = (char **) flags->argv;
    char flog[50] = "log_";             /*Nombre del archivo de log*/

    /*Inicializacion semaforo en 1*/
    sem_init(&sem , 0, 1);
    sem_init(&semf, 0, 1);
    sem_init(&seml, 0, 1);
    
    /*Creamos la lista de tickets*/
    tickets = create_list();
    /*procedimiento que obtiene los valores generales*/
    if (llamadaC(argc, argv, &nombre, &max, &gas, &resp, &entrada) < 0) {
        exit(1);
    }

    /*Arma el nombre del archivo de log*/
    strcat(flog, nombre);
    strcat(flog, ".txt");
    /*Abre el archivo de log*/
    if ((out = fopen(flog, "w")) == NULL) {

        errorFile(__LINE__);
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
