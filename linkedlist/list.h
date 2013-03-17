#ifndef T
#define T int  //Luego se cambia para el T que se necesite
#endif

typedef struct box box;

typedef struct list *list;

list create_list();

void destroy_list();

box *create_box(T elem);

int add(list *l, T elem);

int remove_elem(list *l, T elem);

T *get(list *l, T elem);

int get_size(list *l);

int is_empty(list *l);
