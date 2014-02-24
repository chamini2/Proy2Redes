#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extra.h"
#define DEFAULT_CHAR "\033[0m"
#define UNDER_CHAR "\033[4m"
#define BOLD_CHAR "\033[1m"

/**
 * Constructor del tipo distr.
 * @param  nombre nombre de distr.
 * @param  DNS    DNS de distr
 * @param  puerto puerto de distr
 * @param  pr     prioridad (tiempo de respuesta) de distr
 * @return        Devuelve el elemento.
 */
distr create_distr(char *nombre, char *DNS, int puerto, int pr) {
    distr d;
    char* temp;
    int size;

    /*Pide memoria para la estrucura de distribuidor*/
    if ((d = (distr) malloc(sizeof(struct distr))) == NULL) {
        errorMem();
        return NULL;
    }

    /*Copiamos los atributos para evitar errores de apuntadores*/
    size = strlen(nombre);
    if ((temp = (char *) malloc((size + 1) * sizeof(char))) == NULL) {
        errorMem();
    }
    strcpy(temp, nombre);
    d->nombre = temp;

    size = strlen(DNS);
    if ((temp = (char *) malloc((size + 1) * sizeof(char))) == NULL) {
        errorMem();
    }
    strcpy(temp, DNS);
    d->DNS = temp;

    d->puerto = puerto;
    d->pr = pr;
}

/**
 * Imprime la ayuda de la Bomba.
 */
 void imprimirHB() {

    printf(BOLD_CHAR"NOMBRE\n"DEFAULT_CHAR);

    printf("     "BOLD_CHAR"bomba"DEFAULT_CHAR" - simula una bomba (cliente) a "
           "conectarse con un centro (servidor).\n\n");

    printf(BOLD_CHAR"SINTAXIS\n"DEFAULT_CHAR);

    printf(BOLD_CHAR"bomba ["DEFAULT_CHAR" -h "BOLD_CHAR"|"DEFAULT_CHAR);
    printf(" –n nombreBomba –cp capacidadMaxima –i inventario –c consumo");
    printf(" –fc FicheroCentros "BOLD_CHAR"]\n\n"DEFAULT_CHAR);

    printf(BOLD_CHAR"FLAGS\n"DEFAULT_CHAR);

    printf("     -h: Ayuda, explica el uso del comando.\n\n");

    printf("     -n: Indica el nombre de la bomba \n");
    printf("              Este flag viene acompanado obligatoriamente por "
           "una cadena de caracteres.\n");

    printf("     -cp: Indica la capacidad maxima de almacenamiento de la "
           "bomba\n");
    printf("              Este flag debe estar acompañado por un entero entre "
           "38.000 y 380.000\n\n");

    printf("     -i: Indica el inventario inicial de la bomba.\n");
    printf("               Este flag debe estar acompañado por un entero entre"
               "0 y capacidad maxima.\n\n");

    printf("     -c: Indica el consumo de gasolina en litros por minuto de "
           "la bomba.\n");
    printf("               Este flag debe estar acompañado por un entero entre"
               "0 y 1000.\n\n");

    printf("     -fc: Nombre del fichero que contiene en cada linea el nombre"
           " de un centro de distribucion asi como las \n          direcciones"
           " (DNS) de la maquina donde corre su servicio y el numero de puerto "
           "en el que corre el \n          servicio en esa maquina, separados "
           "entre si por un \"&\"\n");
    printf("               Este flag debe estar acompañado por una cadena de caracteres"
                   "indicando path absoluto o relativo del archivo.\n\n");

    printf(BOLD_CHAR"AUTOR\n"DEFAULT_CHAR);

    printf("     Escrito por Juan Arocha y Matteo Ferrando.\n");
}

/**
 * Imprime el uso de la Bomba.
 */
void imprimirEB() {
    printf("uso: "BOLD_CHAR"bomba ["DEFAULT_CHAR" -h "BOLD_CHAR"|"DEFAULT_CHAR);
    printf(" –n nombreBomba –cp capacidadMaxima –i inventario –c consumo");
    printf(" –fc FicheroCentros "BOLD_CHAR"]\n"DEFAULT_CHAR);
}

/**
 * Imprime la ayuda del Centro.
 */
void imprimirHC() {

    printf(BOLD_CHAR"NOMBRE\n"DEFAULT_CHAR);

    printf("     "BOLD_CHAR"centro"DEFAULT_CHAR" - simula un centro de "
           "distribucion (servidor) a conectarse con una bomba (cliente).\n\n");

    printf(BOLD_CHAR"SINTAXIS\n"DEFAULT_CHAR);

    printf(BOLD_CHAR"centro ["DEFAULT_CHAR" -h "BOLD_CHAR"|"DEFAULT_CHAR);
    printf(" –n nombreCentro –cp capacidadMaxima –i inventario –t tiempo");
    printf(" –s suministro -p puerto "BOLD_CHAR"]\n"DEFAULT_CHAR);

    printf(BOLD_CHAR"FLAGS\n"DEFAULT_CHAR);

    printf("     -h: Ayuda, explica el uso del comando.\n\n");

    printf("     -n: Indica el nombre del centro \n");
    printf("              Este flag viene acompanado obligatoriamente por "
           "una cadena de caracteres.\n");

    printf("     -cp: Indica la capacidad maxima de almacenamiento del "
           "centro\n");
    printf("               Este flag debe estar acompañado por un entero entre "
           "38.000 y 3.800.000\n\n");

    printf("     -i: Indica el inventario inicial del centro.\n");
    printf("               Este flag debe estar acompañado por un entero entre"
               "0 y capacidad maxima.\n\n");

    printf("     -t: Tiempo de respuesta del centro en minutos.\n");
    printf("               Este flag debe estar acompañado por un entero entre"
               "0 y 180.\n\n");

    printf("     -s: Indica el suminstro de gasolina en litros por minuto del "
           "centro.\n");
    printf("               Este flag debe estar acompañado por un entero entre"
               "0 y 10000.\n\n");

    printf("     -p: Puerto por el cual se escuchan peticiones.\n");
    printf("               Este flag debe estar acompañado por un entero"
                   " para un puerto valido.\n\n");

    printf(BOLD_CHAR"AUTOR\n"DEFAULT_CHAR);

    printf("     Escrito por Juan Arocha y Matteo Ferrando.\n");
}

/**
 * Imprime el uso del Centro.
 */
void imprimirEC() {
    printf("uso: "BOLD_CHAR"centro ["DEFAULT_CHAR" -h "BOLD_CHAR"|"DEFAULT_CHAR);
    printf(" –n nombreCentro –cp capacidadMaxima –i inventario –t tiempo");
    printf(" –s suministro -p puerto "BOLD_CHAR"]\n"DEFAULT_CHAR);
}

/**
 * Analisis de la llamada a la Bomba.
 * @param  argc   numero de argumentos en la llamada.
 * @param  argv   arreglo de strings de argumentos.
 * @param  nombre donde colocara el nombre de la Bomba (in-out).
 * @param  fich   donde colocara el fichero de la Bomba(in-out).
 * @param  max    donde coloclara la capacidad maxima de la Bomba (in-out).
 * @param  inv    donde colocara el inventario de la Bomba (in-out).
 * @param  con    donde colocara el consumo por minuto de la Bomba (in-out).
 * @return        un valor indicando si la llamada fue correcta.
 */
int llamadaB (int argc, char **argv, char **nombre, char **fich, int *max, int *inv, int *con) {

    int i, flags;

    switch (argc) {

        case 1:            /*sin flags*/
            imprimirEB();
            return -1;
        case 2:            /*-h ayuda*/
            if (strcmp(argv[1], "-h") == 0) {
                imprimirHB();
            } else {
                imprimirEB();
            }
            return -1;
        default:           /*flags*/
            /*si no tiene el numero de flags adecuado*/
            if (argc != 11) {
                imprimirEB();
                return -1;
            }

            for (i = 1; i < argc; ++i) {

                /*Si ya reviso esta posicion*/
                if (argv[i] == NULL) {
                    continue;
                }
                
                /*Si consigue -n nombre*/
                if (strcmp(argv[i], "-n") == 0) {
                    if ((*nombre = (char *) malloc((strlen(argv[i+1]) + 1) * sizeof(char))) == NULL) {

                        return errorMem();
                    }
                    strcpy(*nombre,argv[i+1]);

                /*Si consigue -cp capacidad maxima*/
                } else if (strcmp(argv[i], "-cp") == 0) {
                    *max = atoi(argv[i+1]);

                /*Si consigue -i inventario*/
                } else if (strcmp(argv[i], "-i") == 0) {
                    *inv = atoi(argv[i+1]);

                /*Si consigue -c consumo de gasolina*/
                } else if (strcmp(argv[i], "-c") == 0) {
                    *con = atoi(argv[i+1]);

                /*Si consigue -fc fichero de centros*/
                } else if (strcmp(argv[i], "-fc") == 0) {
                    if ((*fich = (char *) malloc((strlen(argv[i+1]) + 1) * sizeof(char))) == NULL) {

                        return errorMem();
                    }
                    strcpy(*fich,argv[i+1]);

                /*Si no es un flag valido*/
                } else {

                    imprimirEB();
                    return -1;
                }

                /*Indico que ya conoce los valores de estas*/
                argv[i] = argv[i+1] = NULL;
            }
    }

    return 0;
}

/**
 * Analisis de la llamada a la Bomba.
 * @param  argc   numero de argumentos en la llamada.
 * @param  argv   arreglo de strings de argumentos.
 * @param  nombre donde colocara el nombre del Centro (in-out).
 * @param  max    donde coloclara la capacidad maxima del Centro (in-out).
 * @param  inv    donde colocara el inventario del Centro (in-out).
 * @param  tiempo donde colocara el tiempo de respuesta del Centro (in-out).
 * @param  sum    donde colocara el suministro por minuto del Centro (in-out).
 * @param  puerto donde colocara el numero de puerto donde correra el servidor (in-out).
 * @return        un valor indicando si la llamada fue correcta.
 */
int llamadaC (int argc, char **argv, char **nombre, int *max, int *inv, int *tiempo, int *sum, int *puerto ) {
    int i, flags;

    switch (argc) {

        case 1:            /*sin flags*/
            imprimirEC();
            return -1;
        case 2:            /*-h ayuda*/
            if (strcmp(argv[1], "-h") == 0) {
                imprimirHC();
            } else {
                imprimirEC();
            }
            return -1;
        default:           /*flags*/
            /*si no tiene el numero de flags adecuado*/
            if (argc != 13) {
                imprimirEC();
                return -1;
            }

            for (i = 1; i < argc; ++i) {

                /*Si ya reviso esta posicion*/
                if (argv[i] == NULL) {
                    continue;
                }

                /*Si consigue -n nombre*/
                if (strcmp(argv[i], "-n") == 0) {
                    *nombre = argv[i+1];

                    /*Si consigue -cp capacidad maxima*/
                } else if (strcmp(argv[i], "-cp") == 0) {
                    *max = atoi(argv[i+1]);

                    /*Si consigue -i inventario*/
                } else if (strcmp(argv[i], "-i") == 0) {
                    *inv = atoi(argv[i+1]);

                    /*Si consigue -t tiempo de respuesta*/
                } else if (strcmp(argv[i], "-t") == 0) {
                    *tiempo = atoi(argv[i+1]);

                    /*Si consigue -s suministro*/
                } else if (strcmp(argv[i], "-s") == 0) {
                    *sum = atoi(argv[i+1]);

                    /*Si consigue -p puerto del servidor*/
                } else if (strcmp(argv[i], "-p") == 0) {
                    *puerto = atoi(argv[i+1]);

                    /*Si no es un flag valido*/
                } else {

                    imprimirEC();
                    return -1;
                }

                /*Indico que ya conoce los valores de estas*/
                argv[i] = argv[i+1] = NULL;
            }
            
    }

    return 0;
}
