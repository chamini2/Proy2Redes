#include <stdio.h>

/*
 * Indica error en memoria de llamada al sistema.
 */
void errorMem(int line) {

    printf("\nError en memoria.\nLinea: %d\n", line);
    exit(1);
}

/*
 * Indica error en hilo de llamada al sistema.
 */
void errorHilo(int line) {

    printf("\nError en hilo.\nLinea: %d\n", line);
    exit(1);
}

/*
 * Indica error en socket de llamada al sistema.
 */
void errorSocket(int line) {

    printf("\nError en socket.\nLinea: %d\n", line);
    exit(1);
}

/*
 * Indica error en archivo de llamada al sistema.
 */
void errorFile(int line) {

    printf("\nError en escritura/lectura de archivos.\nLinea: %d\n", line);
    exit(1);
}
