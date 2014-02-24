#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include "queue.h"
#include "errores.h"
#include "logistica.h"
#include <time.h>

char *md5(char *s) {

    char *arg;
    pid_t pid;
    int pipefd[2], status, i;

    pipe(pipefd);

    /*Proceso hijo*/
    if ((pid = fork()) == 0) {

        close(pipefd[0]);

        dup2(pipefd[1], 1);  // stdout al pipe

        /*Cerramos el file descriptor*/
        close(pipefd[1]);

        /*Memoria para el argumento*/
        if ((arg = malloc(sizeof(char) * (strlen(s) + 3))) == NULL) {

            // errorMem(__LINE__);
            return NULL;
        }

        sprintf(arg, "-s%s", s);

        execl("./md5-c/md5", "md5", arg, NULL);
    } else if (pid < 0) {

        // errorFork(__LINE__);
        return NULL;
    } else {

        if ((arg = malloc(sizeof(char) * 64)) == NULL) {

            // errorMem(__LINE__);
            return NULL;
        }

        memset(arg, '\0', sizeof(char) * 64);

        close(pipefd[1]);
        waitpid(pid, &status, 0);

        if (status == 0) {

            read(pipefd[0], arg, 64);
            close(pipefd[0]);

        } else {

            free(arg);
            // errorWaitPid(__LINE__);
            return NULL;
        }

        close(pipefd[0]);

        return arg;
    }

    return NULL;
}

void gen_random(char *s, const int len) {

    int i;
    srand(time(NULL));

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for ( i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

int main(int argc, char const *argv[])
{
    char *s;

    s = (char *) malloc(sizeof(char) * 64);

    printf("%s", md5("abc"));
    gen_random(s, 64);
    printf("\n%s\n", s);
    printf("%s\n", md5(s));
    sleep(1);

    return 0;
}