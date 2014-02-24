#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include "extra.h"
#include "errores.h"
#define QLENGTH 5

char *nombre = NULL;    /*Nombre de la bomba*/
int gas      = 0;       /*Cantidad de gasolina actual*/
int max      = 0;       /*Cantidad maxima de gasolina*/
int entrada  = 0;       /*Cantidad de gasolina que entra en litros por minuto*/
int resp     = 0;       /*Tiempo de respuesta del centro*/
int tiempo   = 0;       /*Tiempo transcurrido desde que comenzo a operar el centro*/
sem_t sem;              /*Semaforo para control de acceso a 'gas'*/
sem_t semf;             /*Semaforo para control de escritura en el log*/
FILE *out;              /*Arhcivo del log*/

/*
 * Funcion de hilo, aumenta el inventario de gasolina del centro
 */
void *controlGas(){

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
            sem_post(&semf);
        }

        /*signal para liberar 'gas'*/
        sem_post(&sem);
    }

    /*Termina el hilo*/
    exit(0);
}

/**
 * Funcion de hilo, se manejan peticiones hechas al servidor.
 * @param  file descriptor del socket.
 */
void *manejo_peticion(void *args){

    int newfd =  *((int *) args);   /*Sabemos que args es en realidad un apuntador a int*/
    char conex[256];
    char *nomB;
    char *pet;
    char res[4];

    free(args);

    if (read(newfd, conex, 256) < 4) {

        close(newfd);
        errorFile(__LINE__);
        pthread_exit(NULL);
    }

    /*Obtenemos el nombre de la bomba*/
    nomB = strtok(conex, "&");

    /*Obtenemos la peticion*/
    pet = strtok(NULL, "&");

    /*Si es una peticion de tiempo de respuesta*/
    if ((strcmp(pet, "t") == 0) || (strcmp(pet, "T") == 0)) {

        sprintf(res, "%d", resp);

        /*Escribe el tiempo de respuesta al cliente*/
        if (write(newfd, res, 4) < 1) {

            close(newfd);
            errorFile(__LINE__);
            pthread_exit(NULL);
        }

        pthread_exit(NULL);
    }

    /*Si es una peticion de gasolina*/
    if ((strcmp(pet, "g") == 0) || (strcmp(pet, "G") == 0)) {

        /*wait para accesar a 'gas'*/
        sem_wait(&sem);

        /*Si tengo suficiente gasolina*/
        if (gas >= CARGA){

            sprintf(res, "1");

            /*Escribe que SI enviara la gasolina al cliente*/
            if (write(newfd, res, 2) < 1) {

                close(newfd);
                errorFile(__LINE__);
                pthread_exit(NULL);
            }

            gas -= CARGA;

            /*Escritura en log*/
            sem_wait(&semf);

            fprintf(out, "Suministro: %d, %s, Positiva, %d\n", tiempo, nomB, gas);

            /*Si se vacio el tanque*/
            if (gas == 0) {

                fprintf(out, "Tanque vacio: %d\n", tiempo);
            }

            sem_post(&semf);

        /*Si no hay suficiente gasolina*/
        } else {

            sprintf(res, "0");

            /*Escribe que NO enviara la gasolina al cliente*/
            if (write(newfd, res, 2) < 1) {

                close(newfd);
                errorFile(__LINE__);
                pthread_exit(NULL);
            }

            /*Escritura en log*/
            sem_wait(&semf);
            fprintf(out, "Suministro: %d, %s, Negativa, %d\n", tiempo, nomB, gas);
            sem_post(&semf);
        }

        sem_post(&sem);
    }

    close(newfd);

    pthread_exit(NULL);
}

int main(int argc, char **argv) {

    pthread_t controlador;              /*Hilo que controla la entrada de gasolina al centro*/
    pthread_t mensajero;                /*Hilo que responde a las llamadas de las bombas*/
    int puerto;                         /*Puerto a utilizar por el servidor*/
    int fd, newfd;                      /*File descriptor del socket y del socket que recibe la conexion*/
    struct sockaddr_in Bdir, Cdir;      /*Estructuras para los sockets*/
    socklen_t Btam;                     /*Tamaño del 'Bdir'*/
    int *thrArg;                        /*Estructura para el paso de argumentos al hilo*/
    char flog[50] = "log_";             /*Nombre del archivo de log*/

    /*Inicializacion semaforo en 1*/
    sem_init(&sem,0,1);
    sem_init(&semf,0,1);

    /*procedimiento que obtiene los valores generales*/
    if (llamadaC(argc, argv, &nombre, &max, &gas, &resp, &entrada, &puerto) < 0) {
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

    /*Si comenzo vacio el tanque*/
    if (gas == 0) {

        fprintf(out, "Tanque vacio: %d\n", tiempo);
    }

    /*Si comenzo lleno el tanque*/
    if (gas == max) {

        fprintf(out, "Tanque full: %d\n", tiempo);
    }

    /*Creo hilo que controla entrada de gasolina*/
    if (pthread_create(&controlador, NULL, controlGas, NULL) != 0){

        return errorHilo(__LINE__);
    }

    /*El servidor se monta para recibir conexiones*/
    /*Abriendo socket con protocolo TCP*/
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){

        return errorSocket(__LINE__);
    }

    /*Inicializacion de la estructura 'Bdir' para el bind*/
    bzero(&Bdir, sizeof(Bdir));
    Cdir.sin_family = AF_INET;
    Cdir.sin_addr.s_addr = htonl(INADDR_ANY);
    Cdir.sin_port = htons(puerto);

    /*Nombra al socket con bind*/
    if (bind(fd, (struct sockaddr *) &Cdir, sizeof(Cdir)) != 0){

        return errorSocket(__LINE__);
    }

    /*El servidor es abierto a conexiones*/
    if (listen(fd, QLENGTH) < 0){

        return errorSocket(__LINE__);
    }

    /*Ciclo para recibir conexiones*/
    while (tiempo < LIMITE){

        Btam = sizeof(Bdir);
        /*Se aceptan conexiones con el cliente*/
        if ((newfd = accept(fd, (struct sockaddr *) &Bdir, &Btam)) < 0){

            return errorSocket(__LINE__);
        }

        /*Pido memoria para pasar argumento del hilo*/
        if ((thrArg = (int *) malloc(sizeof(int))) == NULL) {

            return errorMem(__LINE__);
        }

        *thrArg = newfd;

        /*Hilo para manejar la peticion entrante*/
        if (pthread_create(&mensajero, NULL, manejo_peticion, (void *) thrArg) != 0){

            return errorHilo(__LINE__);
        }

        /*Para que el sistema recupere los recursos al terminar de ejecutar el hilo*/
        if (pthread_detach(mensajero) != 0){

            return errorHilo(__LINE__);
        }
    }

    /*Cierro el archivo de log*/
    fclose(out);
    /*Espero que atienda las peticiones restantes*/
    exit(0);
}
