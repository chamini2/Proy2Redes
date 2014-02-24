typedef struct pase {

    int tiempo;
    int numero;
    int autenticando; //1 = en proceso, 0 = autenticado
    char *login;
    char *contrasena;
} pase;

#define T pase

int equals_pase(pase *a, pase *b);

#define EQUAL equals_pase
