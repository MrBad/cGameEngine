#include "quad_tree.h"
#include "aabb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define QT_TREE_MAX_OBJECTS 2
// Quad Tree Objects //


QuadTreeObj *quadTreeObjNew(AABB limits, void *data) 
{
	QuadTreeObj *obj = malloc(sizeof(*obj));
	if(!obj) {
		fprintf(stderr, "Cannot create Quad Tree Object\n");
		return NULL;
	}
	obj->limits = limits;
	obj->data = data;
	return obj;
}

void quadTreeObjDelete(QuadTreeObj *obj) 
{
	free(obj);
}

void QuadTreeObjDel(void *obj) {
	quadTreeObjDelete(obj);
}

QuadTree *quadTreeNew(AABB box) 
{
	int i;
	QuadTree *tree = NULL;
	if(!(tree = malloc(sizeof(*tree)))) {
		fprintf(stderr, "Cannot alloc QuadTree\n");
		return false;
	}
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		tree->childs[i] = NULL;
	}
	tree->box = box;
	tree->objects = listNew(NULL);

	return tree;
}

bool quadTreeDelete(QuadTree *tree) 
{
	int i;
	if(!tree) {
		return false;
	}
	// delete it's childs
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		return quadTreeDelete(tree->childs[i]);
	}
	// delete objects attached //
	ListNode *objNode;
	list_foreach(tree->objects, objNode) {
		free(objNode->data);
	}
	// free list and delete objects attached
	listDelete(tree->objects);
	free(tree);
	return true;
}


static int quadTreeGetIndex(QuadTree *node, QuadTreeObj *obj) 
{
	int i;
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		assert(node->childs[i] != NULL);
		if(aabbFitsInAABB(obj->limits, node->childs[i]->box))
			return i;
	}
	// does not fit into any child 
	return -1;
}

void quadTreeSplit(QuadTree *node)
{	
	float x = node->box.minX;
	float y = node->box.minY;
	float maxX = node->box.maxX;
	float maxY = node->box.maxY;
	float halfWidth = (maxX - x) / 2.0f;
	float halfHeight = (maxY - y) / 2.0f;

	node->childs[NE] = quadTreeNew(aabb(x + halfWidth, y + halfHeight, maxX, maxY));
	node->childs[NW] = quadTreeNew(aabb(x, y + halfHeight, x + halfWidth, maxY));
	node->childs[SW] = quadTreeNew(aabb(x, y, x + halfWidth, y + halfHeight));
	node->childs[SE] = quadTreeNew(aabb(x + halfWidth, y, maxX, y + halfHeight));
}


static void printNode(QuadTree *node) 
{
	printf("node: {%8.2f, %8.2f, %8.2f, %8.2f}\n", 
			node->box.minX, node->box.minY, node->box.maxX, node->box.maxY);
}
static void printObj (QuadTreeObj *obj) {
	printf("obj:  {%8.2f, %8.2f, %8.2f, %8.2f}, %s\n", obj->limits.minX, obj->limits.minY, obj->limits.maxX, obj->limits.maxY, (char *)obj->data);
}

bool quadTreeInsertObj(QuadTree *node, QuadTreeObj *obj)
{
	int idx;
	// object cannot fit in this node //
	if(!aabbFitsInAABB(obj->limits, node->box)) {
		printf("!fits\n");
		return false;
	}

	if(node->childs[0] == NULL && node->objects->items < 2) {
		if(listAdd(node->objects, obj)) {
			return true;
		}
		return false;
	}
	else if(node->childs[0] == NULL) {
		quadTreeSplit(node);
		
		ListNode *objNode;
		list_foreach(node->objects, objNode) {
			QuadTreeObj *oldObj = objNode->data;
			idx = quadTreeGetIndex(node, oldObj);
			if(idx == -1) {
				continue;
			} else { 
				if(quadTreeInsertObj(node->childs[idx], oldObj)) {
					listDelNode(node->objects, objNode);
				}
			}
		}
	}
	idx = quadTreeGetIndex(node, obj);
	if(idx == -1) {
		listAdd(node->objects, obj); 
	}
	else {
		quadTreeInsertObj(node->childs[idx], obj);
	}
	return true;

}

void printNodes(QuadTree *node, int depth) {
	if(!node)
		return;
	int i;
	printf("---- NodeDepth: %d\n", depth);
	printNode(node);
	printf("objects: %d\n", node->objects->items);
	ListNode *objNode;
	list_foreach(node->objects, objNode) {
		printObj(objNode->data);
	}
	depth++;
	for(i = 0; i < 4; i++) {
		printNodes(node->childs[i], depth);
	}
	depth--;

}


bool quadTreeInsert(QuadTree *node, AABB limits, void *data) 
{
	QuadTreeObj *obj = quadTreeObjNew(limits, data);
	if(!quadTreeInsertObj(node, obj)) {
		printObj(obj);
		printNode(node);
		quadTreeObjDelete(obj);
		return false;
	}
	return true;
}



void quadTreeFind(QuadTree *node, AABB limits, List *result) {
	printf(".");
	// return if limits does not intersect this quad
	if(!aabbIntersect(&node->box, &limits))	 {
		return;
	}
	// Check objects at this quad level and add them 
	// if they intersect limits
	ListNode *objNode;
	list_foreach(node->objects, objNode) {
		QuadTreeObj *obj = objNode->data;
		if(aabbIntersect(&limits, &obj->limits)) {
			listAdd(result, obj);
		}
	}

	// return if it has no childs //
	if(node->childs[0] == NULL)
		return;
	for(int i = 0; i < 4; i++) {
		quadTreeFind(node->childs[i], limits, result);
	}
}

#ifdef COMPILE_TESTS

// TODO - more testing
void quadTreeTest()
{
	printf("Testing QuadTree\n");
	AABB box = aabb(-10, -10, 10, 10);
	QuadTree *tree = quadTreeNew(box);


	//box = aabb(-20, -20, 20, 20);	
	//assert(quadTreeInsert(tree, box, "Bad Node") == false);

	box = aabb(3, 5, 5, 7);
	assert(quadTreeInsert(tree, box, "First Node"));
	assert(tree->childs[0] == NULL);
	assert(tree->objects->items == 1);
	assert(((AABB *)tree->objects->head->data)->minX == 3);
	assert(((AABB *)tree->objects->head->data)->maxX == 5);

	box = aabb(-5, 2, -3, 4);
	assert(quadTreeInsert(tree, box, "Second Node"));
	assert(tree->childs[0] == NULL);
	assert(tree->objects->items == 2);
	assert(((AABB *)tree->objects->head->data)->minX == 3);	
	assert(((AABB *)tree->objects->head->next->data)->minX == -5);

	// it should split 
	box = aabb(-5, -7, -3, -5);
	assert(quadTreeInsert(tree, box, "Third Node"));


	assert(tree->objects->items == 0);
	assert(tree->childs[NE]->objects->items == 1);
	assert(tree->childs[NW]->objects->items == 1);
	assert(tree->childs[SW]->objects->items == 1);
	assert(tree->childs[SE]->objects->items == 0);

	box = aabb(2, 4, 4, 6);
	assert(quadTreeInsert(tree, box, "Forth Node"));
	
	box = aabb(1, 1, 3, 3);
	assert(quadTreeInsert(tree, box, "Fifth Node"));


	box = aabb(6, 6, 8, 8);
	assert(quadTreeInsert(tree, box, "Sixth Node"));
	
	printf("\n\n\n");
	printNodes(tree, 0);
	printf("\n\n\n");

	List *res = listNew(NULL);
	AABB limits = aabb(-10, -10, 0, 0);
	
	quadTreeFind(tree, limits, res);

	ListNode *objNode;
	list_foreach(res, objNode) {
		QuadTreeObj *obj = objNode->data;
		printObj(obj);
	}
	listDelete(res);
//	assert(strcmp("STOP HERE", "Continue on quaternions, they are fun!!")== 0);
}
#endif
