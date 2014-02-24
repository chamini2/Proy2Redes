#include "extra.h"

typedef struct box box;
typedef struct queue *queue;
typedef box **iterator;

int is_empty(queue q);

int add(queue *q, distr e);

distr get(queue q);

void clear_queue(queue *q);

iterator create_iterator(queue q);

distr next_it(iterator it);

distr prev_it(iterator it);