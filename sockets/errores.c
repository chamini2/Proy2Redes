#include <stdio.h>

/*
 * Indica error en memoria de llamada al sistema.
 */
int errorMem(int line) {

    printf("\nError en memoria.\nLinea: %d\n", line);
    return -1;
}

/*
 * Indica error en hilo de llamada al sistema.
 */
int errorHilo(int line) {

    printf("\nError en hilo.\nLinea: %d\n", line);
    return -2;
}

/*
 * Indica error en socket de llamada al sistema.
 */
int errorSocket(int line) {

    printf("\nError en socket.\nLinea: %d\n", line);
    return -4;
}

/*
 * Indica error en archivo de llamada al sistema.
 */
int errorFile(int line) {

    printf("\nError en escritura/lectura de archivos.\nLinea: %d\n", line);
    return -5;
}