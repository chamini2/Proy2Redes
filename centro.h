struct args {
  int argc;
  char const *argv[];
};

int equals_pase(pase *a, pase *b);

int autenticado(pase *p);

void *control_gas();

int principal(struct args arg);