#include "pase.h"
#ifndef T
  #define T int  //Luego se cambia para el T que se necesite
#endif
#ifndef EQUAL
  #define EQUAL equals
#endif

typedef struct list *list;

typedef struct box box;

typedef struct box *iter;

list create_list();

void destroy_list();

int add_list(list *l, T *elem);

void print_ticket(T *p);

int remove_elem_list(list *l, T *elem);

T *get_list(list *l, T *elem);

int get_size_list(list *l);

int is_empty_list(list *l);

int contains_list(list *l, T *elem);

iter list_iter(list l);

T *next_iter(iter *it);

int has_next_iter(iter it);
