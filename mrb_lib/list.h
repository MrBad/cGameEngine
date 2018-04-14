/**
 * A list implementation
 */
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

/**
 * Prototype of a generic delete function, to be called
 * when you want to automatically delete the data associated with a node
 */
typedef void (*DelFunc) (void *data);

/* A node in the list */
typedef struct ListNode {
    void *data;
    struct ListNode *prev;
    struct ListNode *next;
} ListNode;

/* The list containing nodes */
typedef struct List {
    ListNode *head;     // first element in the list
    ListNode *tail;     // last element in the list
    unsigned int items; // number of items in the list
    DelFunc delFunc;    // ptr to delete function to call when deleting a node
} List;

/**
 * Creates a new list
 *
 * @param delFunc A delete function to call on node data, when node is deleted.
 *        or NULL when this feature is not desired.
 * @return a reference to the new allocated list.
 */
List *listNew(DelFunc delFunc);

/**
 * Destroys the elements in the list and the list itself.
 * If delFunc is defined, it will be called for each element.
 *
 * @param list The list to destroy
 * @return void
 */
void listDelete(List *list);

/**
 * Adds/appends a new element to the list.
 *
 * @param element The element (struct or whatever) data to add to the list.
 * @return true on success, false on error
 */
bool listAdd(List *list, void *element);

/**
 * Deletes a node from the list.
 * If delFunc is defined, it will be called.
 *
 * @param list The list to delete from
 * @param node The ListNode to delete
 * @return true on success. (XXX always true)
 */
bool listDelNode(List *list, ListNode *node);

/**
 * Search for and deletes an element from the list.
 *
 * @param list The list to delete from
 * @param element Element to delete
 * @return true on success, false if element not found in the list
 */
bool listDel(List *list, void *element);

/**
 * Helper for looping in the list
 */
#define list_foreach(list, node) \
    for(node = list->head; node; node = node->next)

/**
 * Helper for looping in the list
 *
 * example:
 * User *obj;
 * ListNode *node;
 * listForEach(list, node, obj) {
 *     printf("%s\n", obj->userName);
 * }
 *
 */
#define listForEach(list, node, obj) \
    for(node = list->head; node && (obj = node->data); node = node->next)

/**
 * Tests the list
 */
void listTest();

#endif

