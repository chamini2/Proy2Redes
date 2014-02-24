#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include "queue.h"
#include "errores.h"

char *nombre  = NULL;     /*Nombre de la bomba*/
int tiempo    = 0;        /*Tiempo transcurrido*/
int gas       = 0;        /*Cantidad de gasolina actual*/
int max       = 0;        /*Capacidad maxima de la bomba*/
int pet       = 1;        /*Numero de peticiones a hacer*/
queue centros = NULL;     /*Cola de prioridad con los centros de distribuicion*/
sem_t sem;                /*Semaforo para control de acceso a 'gas' y 'pet'*/
sem_t semf;               /*Semaforo para control de escritura de archivo*/
FILE *out;                /*Arhcivo del log*/

/**
* Hace una conexion con el centro indicado y retorna su respuesta.
* @param  tipo   tipo de peticion.
* @param  puerto puerto por el cual conectarse con el servidor.
* @param  DNS    DNS del servidor.
* @return        Respuesta del servidor.
*/
int conectar_centro(char tipo, int puerto, char *DNS) {

char respuesta[4];             /*Respuesta del servidor*/
char peticion[256];            /*Mensaje que se enviara al serivdor*/
int fd;                        /*File descriptor del socket*/
struct sockaddr_in Cdir;       /*Estructura para el socket del servidor (centro de distribucion)*/

    /*Si la peticion no es ni de gasolina ni de tiempo de respuesta*/
    if ((tipo != 'G') && (tipo != 'g') && (tipo != 'T') && (tipo != 't')) {

    /*Respuesta negativa*/
        return -1;
    }

    sprintf(peticion, "%s&%c", nombre, tipo);

    /*Abriendo socket con protocolo TCP*/
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

        errorSocket(__LINE__);
        return -1;
    }

    /*Inicializacion de la estructura 'Cdir'*/
    bzero(&Cdir, sizeof(Cdir));
    Cdir.sin_family = AF_INET;
    Cdir.sin_addr = *((struct in_addr *) gethostbyname(DNS)->h_addr);
    Cdir.sin_port = htons(puerto);

    /*Conecto con el servidor*/
    if (connect(fd,(struct sockaddr *) &Cdir, sizeof(Cdir)) < 0) {

        close(fd);
        errorSocket(__LINE__);
        return -1;
    }

    /*Enviamos peticion al servidor*/
    if (write(fd, peticion, 256) < 1) {

        close(fd);
        errorFile(__LINE__);
        return -1;
    }

    /*Esperamos la respuesta*/
    if (read(fd, respuesta, 4) < 1) {

        close(fd);
        errorFile(__LINE__);
        return -1;
    }

    /*Cierro el file descriptor*/
    close(fd);

    return atoi(respuesta);
}

/**
* Analiza el fichero pasado en la llamada del programa.
* @param  fich nombre del fichero.
* @return      Si se hizo correctamente, se retorna la espera minima, sino 0 (false) para indicar falla.
*/
int analizar_fichero(char *fich) {

FILE *fd     = NULL;        /*File descriptor del fichero de centros*/
char buffer[100];           /*Buffer de lectura para el archivo.*/
distr cent;                 /*Variable que representa un centro*/
char *nom, *DNS;            /*Nombre y DNS de centro de distribucion*/
int puerto, respuesta;      /*Puerto y tiempo de respuesta de centro de distribucion*/
int min_resp = MAX_INT;     /*Tiempo de respuesta minimo de los centros de distribucion*/
int i;                      /*Variable de uso generico*/

    /*Abre el archivo*/
    if ((fd = fopen(fich, "r")) == NULL) {
        return errorFile(__LINE__);
    }

    /*Lee el archivo hasta el final*/
    while (fscanf(fd, "%s", buffer) != EOF) {

        nom = strtok(buffer, "&");
        DNS = strtok(NULL, "&");
        puerto = atoi(strtok(NULL, "&"));

        /*Intenta conseguir una conexion con el servidor 5 veces, si no lo logra lo ignora*/
        for (i = 0; i < 5; ++i) {

            /*Para conseguir el tiempo de respuesta de este servidor*/
            if ((respuesta = conectar_centro('t', puerto, DNS)) >= 0) {

                break;
            }
        }

        /*Si no logro conectarse con el servidor*/
        if (i == 5) {

            continue;
        }

        /*Si consegui un nuevo minimo tiempo de respuesta*/
        if (respuesta < min_resp) {

            min_resp = respuesta;
        }

        if((cent = create_distr(nom, DNS, puerto, respuesta)) == NULL) {

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

        envio = conectar_centro('g', cent->puerto, cent->DNS);

        /*Si logro una conexion*/
        if (envio != -1) {
            /*Si se enviara la gasolina*/
            if (envio) {

                sem_wait(&semf);
                fprintf(out, "Peticion: %d, %s, Positiva\n", tiempo, cent->nombre);
                sem_post(&semf);
                break;
            /*Si NO se enviara la gasolina*/
            } else {

                sem_wait(&semf);
                fprintf(out, "Peticion: %d, %s, Negativa\n", tiempo, cent->nombre);
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

    /*Si el tanque se lleno*/
    if (gas == max) {

        fprintf(out, "Tanque full: %d\n", tiempo);
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

    /*Si hubo error en la invocacion del porgrama*/
    if (llamadaB(argc, argv, &nombre, &fich, &max, &gas, &salida) < 0) {
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

    /*Revisa el fichero de centros, si hay error termina el programa.*/
    if ((espera = analizar_fichero(fich)) < 0) {
        return -3;
    }

    /*Si comenzo vacio el tanque*/
    if (gas == 0) {

        fprintf(out, "Tanque vacio: %d\n", tiempo);
    }

    /*Si comenzo lleno el tanque*/
    if (gas == max) {

        fprintf(out, "Tanque full: %d\n", tiempo);
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

    return 0;
}
