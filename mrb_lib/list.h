#ifndef _LIST_H
#define _LIST_H

#include <stdbool.h>

typedef void (*DelFunc)(void *data);

typedef struct ListNode {
	void *data;
	struct ListNode *prev;
	struct ListNode *next;
} ListNode;

typedef struct List {
	ListNode *head;
	ListNode *tail;
	unsigned int items;
	DelFunc delFunc;
} List;


List *listNew(DelFunc delFunc);
bool listDelete(List *list);
bool listAdd(List *list, void *data);
bool listDelNode(List *list, ListNode *node);
bool listDel(List *list, void *element);


#define list_foreach(list, node) \
	for(node = list->head; node; node = node->next)
#define listForeach(list, node, obj) \
	for(node = list->head; node && (obj = node->data); node = node->next)

void listTest();

#endif
