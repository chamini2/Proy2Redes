#define MAX_INT 2147483647
#define MINUTO 100000
#define CARGA 38000
#define LIMITE 480

struct distr {
  char *nombre;
  char *DNS;
  int ticket;
  int pr;
};

typedef struct distr *distr;

distr create_distr(char *nombre, char *DNS, int ticket, int pr);

int llamadaB (int argc, char **argv, char **nombre, char **fich, int *max, int *inv, int *con);

int llamadaC (int argc, char **argv, char **nombre, int *max, int *inv, int *tiempo, int *sum);