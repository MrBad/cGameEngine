#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "list.h"

List *listNew(DelFunc del_func)
{
    List *list;

    if (!(list = malloc(sizeof(*list)))) {
        perror("malloc");
        return NULL;
    }
    list->head = list->tail = NULL;
    list->items = 0;
    list->delFunc = del_func;

    return list;
}

void listDelete(List *list)
{
    ListNode *n;

    while (list->head) {
        n = list->head->next;
        if (list->delFunc)
            list->delFunc(list->head->data);
        free(list->head);
        list->head = n;
    }
}

/**
 * Creates a new node and appends the element (data) to it
 *
 * @param element The element (data) to append to the node
 * @return The new created element
 */
static ListNode *listNodeNew(void *element)
{
    ListNode *n;

    if (!(n = malloc(sizeof(*n)))) {
        perror("malloc");
        return NULL;
    }
    n->data = element;
    n->prev = n->next = NULL;

    return n;
}

bool listAdd(List *list, void *element)
{
    ListNode *n;

    if (!element)
        return false;

    if (!(n = listNodeNew(element)))
        return false;

    if (!list->head) {
        list->head = list->tail = n;
    } else {
        n->prev = list->tail;
        list->tail->next = n;
        list->tail = n;
    }
    list->items++;

    return true;
}

bool listDelNode(List *list, ListNode *node) 
{
    ListNode *prev, *next;

    prev = node->prev;
    next = node->next;

    if (prev)
        prev->next = next;
    else
        list->head = next;

    if (next)
        next->prev = prev;
    else
        list->tail = prev;

    if (list->delFunc)
        list->delFunc(node->data);

    free(node);
    list->items--;

    return true;
}

bool listDel(List *list, void *element) 
{
    ListNode *node;
    void *data;

    listForEach(list, node, data) {
        if (data == element)
            return listDelNode(list, node);
    }

    return false; // not found
}

#ifdef COMPILE_TESTS

typedef struct { 
    const char *name;
    int numPeople;
} City;

void listTest()
{
    City cities[] = {
        {"Bucharest", 100000},
        {"London", 200000},
        {"Munchen", 150000},
        {"Cluj", 15000},
    };

    List *list = listNew(NULL);
    printf("Testing List\n");

    for (unsigned long i = 0; i < sizeof(cities) / sizeof(*cities); i++) {
        listAdd(list, &cities[i]);
    }

    assert(strcmp(((City *)list->head->data)->name, "Bucharest") == 0);
    assert(strcmp(((City *)list->tail->data)->name, "Cluj") == 0);
    assert(list->items == sizeof(cities)/sizeof(*cities));
    listDelNode(list, list->head);
    assert(list->items == 3);
    assert(strcmp(((City *)list->head->data)->name, "London") == 0);
    listDelNode(list, list->head);
    assert(list->items == 2);
    listDelNode(list, list->head);
    assert(list->items == 1);
    assert(list->head == list->tail);
    listDelete(list);
}

#endif

