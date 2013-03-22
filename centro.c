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

        if (aux == NULL) {

          break;
        }

        /*Si ya no esta autenticado*/
        if ((aux->tiempo + 60 <= tiempo) && (!aux->autenticando)) {
            
            printf("%s | %d - %d", aux->login, aux->tiempo, tiempo);
            remove_elem_list(&tickets, aux);
            continue;
        }

        /*Esta autenticado*/
        if (equals_pase(aux, p)) {
            
            elem = aux;
            break;
        }
    }
    sem_post(&seml);

    if (aux == NULL) {

      return -1;
    }
    /*Si lo consigo actualizo*/
    if (elem != NULL) {
        if (elem->autenticando) {

          return -2;
        }

        elem->tiempo = tiempo;
        return 1;
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

    sem_wait(&sem);

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

        sem_post(&sem);
        return &result;
    /*Si esta en proceso de una autenticacion*/
    } else if (result == -2) {

        sem_post(&sem);
        return &result;
    }

    /*Escritura en log*/
    sem_wait(&semf);
    fprintf(out, "Peticion: %d, %s, Autenticado\n", tiempo, nomB);
    printf("Peticion: %d, %s, Autenticado\n", tiempo, nomB);
  sem_post(&semf);

  result = 0;

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
    char *string;
    char *resp;
    pase *p;

    sem_wait(&sem);

    if ((string = (char *) malloc(128 * sizeof(char))) == NULL) {

        result = NULL;
        errorMem(__LINE__);
    }

    if ((result = (char *) malloc(128 * sizeof(char))) == NULL) {

        free(string);
        result = NULL;
        errorMem(__LINE__);
    }

    if ((p = (pase *) malloc(sizeof(pase))) == NULL) {

        free(string);
        result = NULL;
        errorMem(__LINE__);
    }

    gen_random(string, 64);
    resp = md5(string);

    ++ticket;
    sprintf(result, "%d&%s", ticket,string);
  
    p->numero       = ticket;
    p->login        = (char*) malloc(sizeof(char)*(strlen(*argp)+1));
    if (p->login == NULL) {

      errorMem(__LINE__);
    }
    strcpy(p->login, *argp);
    p->contrasena = (char*) malloc(sizeof(char)*(strlen(resp)+1));
    if (p->contrasena == NULL) {

      errorMem(__LINE__);
    }
    strcpy(p->contrasena, resp);
    p->tiempo       = tiempo;
    p->autenticando = 1;
  
    sem_wait(&seml);
    add_list(&tickets, p);
    sem_post(&seml);
  
    sem_post(&sem);
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

    sem_wait(&sem);

    if ((comp = (pase *) malloc(sizeof(pase))) == NULL) {

        errorMem(__LINE__);
    }

    comp->numero       = atoi(strtok(*argp, "&"));
    comp->login        = nomB = strtok(NULL, "&");
    comp->contrasena   = strtok(NULL, "&");
    comp->tiempo       = tiempo;
    comp->autenticando = 0;


    sem_wait(&seml);
    aux = get_list(&tickets, comp);
    sem_post(&seml);

    if (strcmp(comp->contrasena, aux->contrasena) == 0) {
        
        aux->autenticando = 0;
        aux->tiempo       = tiempo;
        
        sem_wait(&semf);
        fprintf(out, "Autenticacion: %d, %s, Exitosa\n", tiempo, aux->login);
        printf("Autenticacion: %d, %s, Exitosa\n", tiempo, aux->login);
        sem_post(&semf);

        result = 1;
    } else {
        sem_wait(&semf);
        fprintf(out, "Autenticacion: %d, %s, Fallida\n", tiempo, aux->login);
        printf("Autenticacion: %d, %s, Fallida\n", tiempo, aux->login);
        sem_post(&semf);

        result = -1;
    }

    sem_post(&sem);
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
