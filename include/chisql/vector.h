#ifndef __VECTOR_H_
#define __VECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct vector_s {
   size_t size, max_size;
   void **data;
   void (*free) (void *); /* free function */
} vector_t;

vector_t *vector(size_t init_size);
vector_t *vector_withData(size_t size, ...);

vector_t *vector_push(vector_t *vector, void *elem);
void *vector_pop(vector_t *vector);

void *vector_getAt(vector_t *vector, size_t i);
void vector_setAt(vector_t *vector, size_t i, void *data);

size_t vector_size(vector_t *vector);

void vector_setFreeFunc(vector_t *vector, void (*free) (void *));

void vector_print(vector_t *vector, void (*print) (void *));
void vector_printStrings(vector_t *vector);

void vector_clear(vector_t *vector);
void vector_free(vector_t *vector);

#endif
