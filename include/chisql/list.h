#ifndef CHISQL_LIST_H_
#define CHISQL_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

typedef struct ListNode_s {
   void *data;
   struct ListNode_s *next, *prev;
} ListNode_t;

typedef struct List_s {
   size_t size;
   ListNode_t *front, *back;
   void (*del) (void *);
   pthread_mutex_t lock;
   char *(*toString) (void *);
   int (*compare)(const void *, const void *);
   void (*print) (void *);
   void *(*copy) (void *);
   size_t elem_size;
   const char *name;
} List_t;

void list_init(List_t *, void (*del) (void *));
List_t *list_initWith(void *elem);
List_t *list_initWithN(size_t n, ...);
void list_destroy(List_t *l);

void list_setPrintFunc(List_t *l, void (*print) (void *));
void list_setCompFunc(List_t *l, int (*compare) (const void *, const void *));
void list_setCopyFunc(List_t *l, void *(*copy)(void *));

void *list_findByInt(List_t *l, int (*toInt) (void *), int i);
void *list_findByString(List_t *l, void (*toString) (char *,void *), const char *str);
void *list_findByPointer(List_t *l, void *ptr);

bool list_removeByInt(List_t *l, int (*toInt) (void *), int i);
bool list_removeByString(List_t *l, void (*toString) (char *,void *), const char *str);
void *list_removeByPointer(List_t *list, void *ptr);
void *list_removeNode(List_t *list, ListNode_t *node);
void list_removeNodeDelete(List_t *list, ListNode_t *node);
void list_removeByPointerFree(List_t *list, void *ptr);

void list_print(List_t *l, bool verbose);
void list_printCustom(List_t *l, char * (*toString) (void *), bool freeAfter);

/* removes all elements for which pred returns false. Does not free data. */
void list_filter(List_t *l, bool (*pred) (void *));
/* same as above, but frees data. */
void list_filterDelete(List_t *l, bool (*pred) (void *));
/* same as above, but makes a new list. */
void list_filterNew(List_t *l, bool (*pred) (void *));
/* replaces all elements e in l with f(e). Does not free original. */
void list_map(List_t *l, void *(*f) (void *));
/* same as above, but frees original. */
void list_mapDelete(List_t *l, void *(*f) (void *));
/* same as above, but creates a new list */
List_t list_mapNew(List_t *l, void *(*f) (void *));
/* sorts; requires compare function to be defined */
void list_sort(List_t *l);
/* performs a union of two lists using compare function */
List_t list_union(List_t *l1, List_t *l2);
List_t list_difference(List_t *l1, List_t *l2);
List_t list_intersection(List_t *l1, List_t *l2);

ListNode_t *listNode_init(void *data, ListNode_t *next, ListNode_t *prev);

List_t list_deepCopy(List_t *l);

/* Threadsafe */
bool list_addBack(List_t *l, void *data);
void *list_removeBack(List_t *l);
bool list_addFront(List_t *l, void *data);
bool list_addInOrder(List_t *l, void *data);
void *list_removeFront(List_t *l);
void list_addBetween(List_t *list, void *data, ListNode_t *prev, ListNode_t *next);

#endif
