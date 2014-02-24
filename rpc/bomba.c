#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include "queue.h"
#include "errores.h"
#include "logistica.h"

char  *nombre = NULL;       /*Nombre de la bomba*/
int   tiempo  = 0;          /*Tiempo transcurrido*/
int   gas     = 0;          /*Cantidad de gasolina actual*/
int   max     = 0;          /*Capacidad maxima de la bomba*/
int   pet     = 1;          /*Numero de peticiones a hacer*/
queue centros = NULL;       /*Cola de prioridad con los centros de distribuicion*/
sem_t semt;                 /*Semaforo para el ticket de un elemento de la cola*/
sem_t sem;                  /*Semaforo para control de acceso a 'gas' y 'pet'*/
sem_t semf;                 /*Semaforo para control de escritura de archivo*/
FILE  *out;                 /*Arhcivo del log*/

/*
 * Para el uso de RPC.
 */
int pedir_gasolina_centro(char *host, char *arg) {

    CLIENT *clnt;
    int *res, ret;
  
    clnt = clnt_create(host, LOGISTICA, LOGISTICA_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    res = pedir_gasolina_1(&arg, clnt);
    if (res == NULL) {
        clnt_perror(clnt, "call failed:");
    }

    clnt_destroy(clnt);

    ret = *res;
    //free(res);

    return ret;
}

int autenticar_centro(char *host) {

    CLIENT *clnt;
    int *res, ret;
    char **user, *soluc, *desaf;
    char *login;
    int ticket;

    clnt = clnt_create(host, LOGISTICA, LOGISTICA_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    user = pedir_desafio_1(&nombre, clnt);
    ticket = atoi(strtok(*user, "&"));
    desaf = strtok(NULL, "&");

    /*Se calcula la solucion para el desafio*/
    soluc = md5(desaf);

    login = (char *) malloc(128 * sizeof(char));
    if (login == NULL) {

        errorMem(__LINE__);
        exit(1);
    }

    sprintf(login, "%d&%s&%s", ticket, nombre, soluc);
  
    res = autenticar_1(&login, clnt);

    clnt_destroy(clnt);

    ret = *res;
    //free(res);
    //free(user);

    return ret;
}

int pedir_tiempo_centro(char *host) {

    CLIENT *clnt;
    int *res, ret;
    void *pvoid;

    clnt = clnt_create(host, LOGISTICA, LOGISTICA_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    res = pedir_tiempo_1(pvoid, clnt);
    if (res == NULL) {
        clnt_perror(clnt, "call failed:");
    }

    clnt_destroy(clnt);

    ret = *res;
    //free(res);

    return ret;
}

/**
 * Analiza el fichero pasado en la llamada del programa.
 * @param  fich nombre del fichero.
 * @return      Si se hizo correctamente, se retorna la espera minima, sino 0 (false) para indicar falla.
 */
int analizar_fichero(char *fich) {

    FILE *fd = NULL;            /*File descriptor del fichero de centros*/
    char buffer[100];           /*Buffer de lectura para el archivo.*/
    distr cent;                 /*Variable que representa un centro*/
    char *nom, *DNS;            /*Nombre y DNS de centro de distribucion*/
    int ticket, respuesta;      /*Ticket y tiempo de respuesta de centro de distribucion*/
    int min_resp = MAX_INT;     /*Tiempo de respuesta minimo de los centros de distribucion*/

    /*Abre el archivo*/
    if ((fd = fopen(fich, "r")) == NULL) {
        errorFile(__LINE__);
    }

    /*Lee el archivo hasta el final*/
    while (fscanf(fd, "%s", buffer) != EOF) {

        nom = strtok(buffer, "&");
        DNS = strtok(NULL, "&");
        ticket = -1;

        respuesta = pedir_tiempo_centro(DNS);

        /*Si consegui un nuevo minimo tiempo de respuesta*/
        if (respuesta < min_resp) {

            min_resp = respuesta;
        }

        if ((cent = create_distr(nom, DNS, ticket, respuesta)) == NULL) {

            return -1;
        }

        if (!add(&centros, cent)) {

            free(cent);
        }
    }

    /*Si no logro conectarse nunca*/
    if (is_empty(centros)) {

        return 0;
    }

    return min_resp;
}

int autenticarse(distr cent) {

    int i, envio;
    char *login;

    if ((login = (char *) malloc(sizeof(char)*128)) == NULL) {

        errorMem(__LINE__);
        return -1;
    }

    /*Para intentar autenticar maximo 5 veces*/
    for (i = 0; i < 5; ++i) {

        sprintf(login, "%d&%s", cent->ticket, nombre);


        envio = pedir_gasolina_centro(cent->DNS, login);

        /*Si no estaba correctamente autenticado*/
        if (envio == -1) {

            sem_wait(&semt);
            cent->ticket = autenticar_centro(cent->DNS);
            sem_post(&semt);

            /*Si se autentico exitosamente*/
            if (cent->ticket != -1) {

                sem_wait(&semf);
                fprintf(out, "Autenticacion: %d, %s, Exitosa\n", tiempo, cent->nombre);
                printf("Autenticacion: %d, %s, Exitosa\n", tiempo, cent->nombre);
                sem_post(&semf);

            /*Si no logro autenticarse*/
            } else {

                sem_wait(&semf);
                fprintf(out, "Autenticacion: %d, %s, Fallida\n", tiempo, cent->nombre);
                printf("Autenticacion: %d, %s, Fallida\n", tiempo, cent->nombre);
                sem_post(&semf);
            }
        /*Si hay una autenticacion en proceso*/
        } else if (envio == -2) {

            if (i == 5) {
                return -1;
            }

            usleep(5 * MINUTO);
        /*Si obtuvo una respuesta*/
        } else {

            break;
        }
    }

    return envio;
}

/**
 * Funcion de hilo, donde se hacen las peticiones de gasolina a los servidores.
 */
void *pedir_gas() {

    int respuesta, envio;          /*Tiempo de respuesta del centro y si enviara la gasolina*/
    iterator it = NULL;            /*Iterador sobre la cola de prioridad*/
    distr cent;                    /*Para el chequeo de la cola*/

    /*Itera sobre todos los centros hasta conseguir uno disponible*/
    for (cent = next_it(it = create_iterator(centros)); ; cent = next_it(it)) {

        /*Si ya reviso todos los elementos, volvemos a comenzar*/
        if (cent == NULL) {

            usleep(5 * MINUTO);
            it = create_iterator(centros);
            continue;
        }

        envio = autenticarse(cent);

        /*Si obtuvo una respuesta*/
        if (envio != -1) {

            /*Si se enviara la gasolina*/
            if (envio) {

                sem_wait(&semf);
                fprintf(out, "Peticion: %d, %s, Positiva\n", tiempo, cent->nombre);
                printf("Peticion: %d, %s, Positiva\n", tiempo, cent->nombre);
                sem_post(&semf);
                break;

            /*Si NO se enviara la gasolina*/
            } else {

                sem_wait(&semf);
                fprintf(out, "Peticion: %d, %s, Negativa\n", tiempo, cent->nombre);
                printf("Peticion: %d, %s, Negativa\n", tiempo, cent->nombre);
                sem_post(&semf);
            }
        }
    }

    /*Tiempo de respuesta en llegar la gasolina*/
    respuesta = cent->pr;
    usleep(respuesta * MINUTO);

    /*Wait para accesar a 'gas' y 'pet'*/
    sem_wait(&sem);

    /*Agrega la carga del envio e indica que la peticion fue atendida*/
    gas += CARGA;
    --pet;

    sem_wait(&semf);
    fprintf(out, "Llegada de la gandola: %d, %d\n", tiempo, gas);
    printf("Llegada de la gandola: %d, %d\n", tiempo, gas);

    /*Si el tanque se lleno*/
    if (gas == max) {

        fprintf(out, "Tanque full: %d\n", tiempo);
        printf("Tanque full: %d\n", tiempo);
    }
    sem_post(&semf);

    /*Signal para liberar 'gas' y 'pet'*/
    sem_post(&sem);

    pthread_exit(NULL);
}

int main(int argc, char **argv) {

    char *fich;                   /*Nombre del fichero con lista de centros*/
    int salida    = 0;            /*Consumo en litros por minuto de gasolina*/
    int espera    = MAX_INT;      /*Tiempo de espera minimo*/
    int tanque;                   /*Estado del tanque al transcurrir el tiempo minimo de espera*/
    pthread_t mensajero;          /*Hilo que hace peticiones de gasolina*/
    char flog[50] = "log_";       /*Nombre del archivo de log*/

    /*Inicializa semaforos en 1*/
    sem_init(&sem, 0, 1);
    sem_init(&semf, 0, 1);
    sem_init(&semt, 0, 1);

    /*Si hubo error en la invocacion del porgrama*/
    if (llamadaB(argc, argv, &nombre, &fich, &max, &gas, &salida) < 0) {
        return -1;
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

    /*Revisa el fichero de centros, si hay error termina el programa.*/
    if ((espera = analizar_fichero(fich)) < 0) {
        return -3;
    }

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

    /*Mientras no pasen las 8 horas*/
    while (tiempo < LIMITE) {

        ++tiempo;
        /*Duerme por un minuto (0.1 seg)*/
        usleep(MINUTO);

        /*Wait para accesar a 'gas' y 'pet'*/
        sem_wait(&sem);

        /*Si no se ha acabado la gasolina*/
        if (gas - salida > 0) {

            gas -= salida;
        /*Si se acabo la gasolina*/
        } else if (gas > 0) {

            gas = 0;

            sem_wait(&semf);
            fprintf(out, "Tanque vacio: %d\n", tiempo);
            printf("Tanque vacio: %d\n", tiempo);
            sem_post(&semf);
        }

        /*El estado del tanque al transcurrir el tiempo minimo de espera*/
        tanque = (gas - espera * salida);
        /*Si estara vacio para ese momento, le colocamos '0'*/
        if (tanque < 0) {

            tanque = 0;
        }

        /*Si hay espacio suficiente en el inventario*/
        while (max - tanque >= CARGA * pet) {

            /*Aumenta el numero de peticiones a hacer*/
            ++pet;

            /*Crea un hilo que maneje la peticion*/
            if (pthread_create(&mensajero, NULL, pedir_gas, NULL) != 0) {

                errorHilo(__LINE__);
            }
        }

        /*Signal para liberar 'gas' y 'pet'*/
        sem_post(&sem);
    }

    /*Cierra el archivo de log*/
    fclose(out);

    return 0;
}
