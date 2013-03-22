#include <stdio.h>
#include <stdlib.h>
#include "list.h"



/*Struct para la caja de info de la lista*/
struct box {
  T *info;
  struct box *next, *prev;
};

/*Struct del tipo list*/
struct list {
  box *first, *last;
  int size;
};

/*Funcion comparadora de pases*/
int equals_pase(pase *a, pase *b) {
      
    if ((a->numero == b->numero) && (strcmp(a->login, b->login) == 0)) {
        return 1;
    }

    return 0;
}

/*Crea una lista inicializada*/
list create_list() {

  list l;

  l = (list) malloc(sizeof(struct list));

  if (l == NULL) {

    //errorMem(__LINE__);
    return NULL;
  }

  l->first = NULL;
  l->last = NULL;
  l->size = 0;

  return l;
}


/*Destruye una lista*/
void destroy_list(list *l) {

  box *it, *aux;

  if (l == NULL) {

    return;
  }

  it = (*l)->first;

  while (it != NULL) {

    aux = it;
    free(aux->info);
    it = it->next;
    free(aux);
  }

  free(l);

  *l = NULL;

}

/*Crea una caja de la lista*/
box *create_box(T *elem) {

  box *new;

  if (elem == NULL) {

    return NULL;
  }

  new = (box*)malloc(sizeof(struct box));

  if (new == NULL) {

    //errorMem(__LINE__);
    return NULL;
  }

  new->info = elem;
  new->next = NULL;
  new->prev = NULL;

  return new;

}


/*Agrega un elemento*/
int add_list(list *l, T *elem) {
  
  box *new;

  if (l == NULL) {
    *l = create_list();
  }

  if (elem == NULL) {

    return 0;
  }

  /*Creamos el elemento*/
  new = create_box(elem);
 
  /*En caso de que no se pudo crear*/
  if (new == NULL) {

    return 0;
  }
  
  /*Si la lista esta vacia, todo apunta al elemento*/
  if ((*l)->size == 0) {

    (*l)->first = new;
    (*l)->last = new;
  /*Si no esta vacia, se agrega al final*/
  } else {

    (*l)->last->next = new;
    new->prev = (*l)->last;
    (*l)->last = new;
  }



  ++(*l)->size;
  return 1;
}

/*Elimina el elemento pedido*/
int remove_elem_list(list *l, T *elem) {

  box *it;

  it = (*l)->first;

  if (l == NULL) {

    return 0;
  }

  if (elem == NULL) {

    return 0;
  }

  /*Se recorre la lista mientras no se encuentre*/
  while (it != NULL && !EQUAL(elem, it->info)) {

    it = it->next;
  }

  /*Si no se encontro*/
  if (it == NULL) {

    return 0;
  }

  /*Si es el primero*/
  if (it == (*l)->first) {

    (*l)->first = (*l)->first->next;
  /*Si es el ultimo*/
  } else if (it == (*l)->last) {

    (*l)->last = (*l)->last->prev;
  /*Si esta en el medio*/
  } else {

    it->prev->next = it->next;
    it->next->prev = it->prev;
  }

  free(it->info);
  free(it);
  --((*l)->size);
  return 1;
}


/*Obtiene el apuntador al elemento tipo T*/
T *get_list(list *l, T *elem) {

  box *it;
  T *e;

  if (l == NULL) {
    printf("l NULL\n");
    return NULL;
  }

  if (elem == NULL) {
    printf("elem NULL\n");
    return NULL;
  }

  it = (*l)->first;

  
  /*Se recorre la lista mientras no se encuentre*/
  while (it != NULL && !EQUAL(elem, it->info)) {
    
    it = it->next;
  }

  /*Si no se encontro*/
  if (it == NULL) {

    return NULL;
  }

  e = it->info;
  return e;
}


/*Obtiene el tamaño*/
int get_size_list(list *l) {

  if (l == NULL) {

    return 0;
  }

  return (*l)->size;
}

/*True si l esta vacia, false si no*/
int is_empty_list(list *l) {

  if (l == NULL) {

    return 1;
  }

  return (*l)->size == 0;
}


void print_list(list *l) {

  box *it;

  it = (*l)->first;

  if (l == NULL) {

    return;
  }
  
  printf("LISTA:");
  while (it != NULL) {

    it = it->next;
  }

  printf("\n");
}


int contains_list(list *l, T *elem) {

  box *it;

  if (l == NULL) {

    return 0;
  }

  if (elem == NULL) {

    return 0;
  }

  it = (*l)->first;

  while (it != NULL) {

    if (EQUAL(it->info, elem)) {

      return 1;
    }
  }

  return 0;
}


iter list_iter(list l) {

  if (l == NULL) {
    return NULL;
  }

  return l->first;
}

T *next_iter(iter *it) {

  T *elem;

  if ((it == NULL) || (*it == NULL)) {
    return NULL;
  }

  elem = (*it)->info;

  *it = (*it)->next;

  return elem;
}

int has_next_iter(iter it) {

  return it != NULL;
}
