#include "quad_tree.h"
#include "aabb.h"
#include "vec2f.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string.h>

// Maximum number of objects in a node 
// before splitting node //
#define QT_TREE_MAX_OBJECTS 2


static bool quadTreeExpand(QuadTree *tree, AABB newLimits);

//
// Surface functions //
//
// creates a surface
QTObject *objectNew(AABB limits, void *data) 
{
	QTObject *obj = malloc(sizeof(*obj));
	if(!obj) {
		perror("malloc");
		return NULL;
	}
	obj->limits = limits;
	obj->data = data;
	obj->tree = NULL;
	obj->node = NULL;
	return obj;
}


// destroy surface
void objectDelete(QTObject *obj) 
{
	free(obj);
}


static void printObject(QTObject *obj)
{
	printf("surf: {%8.2f, %8.2f, %8.2f, %8.2f}, %p\n",
			obj->limits.minX, obj->limits.minY,
			obj->limits.maxX, obj->limits.maxY,
			obj->data);
}


//
// Quad Tree Nodes
//

// creates a new Quad Tree node //
static QTNode *qtNodeNew(AABB limits) 
{
	int i;
	QTNode *node;
	node = malloc(sizeof(*node));
	if(!node) {
		perror("malloc");
		return NULL;
	}
	node->parent = NULL;
	node->limits = limits;
	node->objects = arrayNew();

	for(i = 0; i < QT_NUM_CHILDS; i++) {
		node->childs[i] = NULL;
	}

	return node;
}


static void qtNodeDelete(QTNode *node, bool recurse)
{
	int i;
	QTObject *obj;
	if(recurse && node->childs[NE]) {
		for(i = 0; i < QT_NUM_CHILDS; i++) {
			qtNodeDelete(node->childs[i], recurse);
		}
	}
	arrayForeach(node->objects, obj, i) {
		objectDelete(obj);
	}
	arrayDelete(&node->objects);
	free(node);
}


// check if this obj fits into one of this node childs
// and return an index of which child belongs, or -1
// if does not fit
static int qtNodeGetIndex(QTNode *node, QTObject *obj)
{
	int i;
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		assert(node->childs[i] != NULL);
		if(aabbFitsInAABB(obj->limits, node->childs[i]->limits))
			return i;
	}
	// does not fit into any child
	return -1;
}

// Split the tree in 4 quadrants, and attach them
static void qtNodeSplit(QTNode *node)
{
	float x = node->limits.minX;
	float y = node->limits.minY;
	float maxX = node->limits.maxX;
	float maxY = node->limits.maxY;
	float halfWidth = (maxX - x) / 2.0f;
	float halfHeight = (maxY - y) / 2.0f;
	int i;

	node->childs[NE] = qtNodeNew(aabb(x + halfWidth, y + halfHeight, maxX, maxY));
	node->childs[NW] = qtNodeNew(aabb(x, y + halfHeight, x + halfWidth, maxY));
	node->childs[SW] = qtNodeNew(aabb(x, y, x + halfWidth, y + halfHeight));
	node->childs[SE] = qtNodeNew(aabb(x + halfWidth, y, maxX, y + halfHeight));

	// set parent
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		node->childs[i]->parent = node;
	}
}

static bool qtNodeAdd(QTNode *node, QTObject *obj) 
{
	int i, idx;
	QTObject *oldObj;
	// object cannot fit in this node //
	if(!aabbFitsInAABB(obj->limits, node->limits)) {
		fprintf(stderr, "---\nSurface out of bounds\n");
		printObject(obj);
		return false;
	}
	// if is leaf and has enough objects room
	if(node->childs[NE] == NULL && node->objects->len < QT_TREE_MAX_OBJECTS) {
		arrayPush(node->objects, obj);
		obj->node = node;
		return true;
	}
	// if is leaf, not splitted and no more room
	else if(node->childs[NE] == NULL) {
		// split the node in 4
		qtNodeSplit(node); 

		// move it's surfaces to childs, if possible
		arrayForeach(node->objects, oldObj, i) {
			idx = qtNodeGetIndex(node, oldObj);	
			if(idx == -1)
				continue;
			else {
				if(qtNodeAdd(node->childs[idx], oldObj)) {
					node->objects->data[i] = NULL;
				}
			}
		}
		arrayCompact(node->objects); // because we nulled some items
	}

	// add original node into one of the childs
	idx = qtNodeGetIndex(node, obj);
	if(idx == -1) {
		arrayPush(node->objects, obj);
		obj->node = node;
	} else {
		return qtNodeAdd(node->childs[idx], obj);
	}
	return true;
}

// climb up the tree and remove empty nodes //
bool qtNodeDeleteUp(QTNode *node) 
{	
	QTNode *n = node->parent;
	int surfacesLeft = 0;
	int i;
	// if all four nodes have no surfaces 
	// and they are leafs
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		surfacesLeft += n->childs[i]->objects->len;
		if(n->childs[i]->childs[NE] != NULL || surfacesLeft > 0)
			return false;
	}
	if(surfacesLeft == 0) {
		for(i = 0; i < QT_NUM_CHILDS; i++) {
			qtNodeDelete(n->childs[i], true);
			n->childs[i] = NULL;
		}
		if(n->objects->len == 0) {
			return qtNodeDeleteUp(n);
		}
	}
	return true;
}

static void printAABB(AABB limits) 
{
	printf("{%8.2f, %8.2f, %8.2f, %8.2f}\n",
			limits.minX, limits.minY, limits.maxX, limits.maxY);
}

bool qtObjectUpdate(QTObject *obj, AABB newLimits) 
{

	QTNode *oldNode, *curr;
	bool ret, found;
	int i;
	oldNode = obj->node;
	assert(obj);	
	// if we still fits in this node bounds, do not move
	// just update the limits
	if(aabbFitsInAABB(newLimits, oldNode->limits)) {
		obj->limits = newLimits;
		return true;
	}
	found = false;
	// else, if we do not fit it anymore, climb up the tree
	// until we can fit inside of a node //
	for(curr = obj->node; curr; curr = curr->parent) {
		if(aabbFitsInAABB(newLimits, curr->limits)) {
			found = true;
			break;
		}
	}

	// probably we reached root
	if(!found) {
		fprintf(stderr, "surfaceUpdate: we reached root and not found any fitting node\n");
		fprintf(stderr, "surfaceUpdate: expanding tree\n");
		////////
		// Expand Tree, added a reference to tree in surface ///
		// Don't like this, but... 
		// How to solve this in an elegant mode?
		////////////////////////
		quadTreeExpand(obj->tree, newLimits);
		////////////////////////
		assert(curr != NULL);
		printAABB(newLimits);
		printAABB(obj->node->limits);
		//exit(1);
		//return false;
	}
	i = arrayIndexOf(oldNode->objects, obj);
	if(i < 0) {
		fprintf(stderr, "We did not found the surface\n");
		return false;
	}
	
	oldNode->objects->data[i] = NULL; // remove
	arrayCompact(oldNode->objects);
	obj->limits = newLimits;
	// insert into newNode //
	ret = qtNodeAdd(curr, obj);
	
	// check if old node is empty and is leaf and if it's neibghors are too, remove all
	if(oldNode->childs[NE] == 0)
		qtNodeDeleteUp(oldNode);
	return ret;	
}


//
// Public functions
//

// All operations are done on Quad Tree
QuadTree *quadTreeNew(AABB limits) 
{
	QuadTree *tree = malloc(sizeof(*tree));
	if(!tree) {
		perror("malloc");
		return NULL;
	}
	tree->root = qtNodeNew(limits);
	tree->items = 0;
	return tree;
}

void quadTreeDelete(QuadTree *tree) 
{
	qtNodeDelete(tree->root, true);
	free(tree);
}


// expands the tree when a newLimits are bigger than the root itself limits
static bool quadTreeExpand(QuadTree *tree, AABB newLimits) 
{
	printf("initial root limits: ");
	printAABB(tree->root->limits);
	printf("requested surface: ");
	printAABB(newLimits);

	AABB doubleLimits;
	AABB oldLimits = tree->root->limits;
	// find the direction where we should grow //
	Vec2f dist = vec2f(newLimits.minX - oldLimits.minX, newLimits.minY - oldLimits.minY);
	Vec2f dir = vec2fNormalize(dist);

	float width = oldLimits.maxX - oldLimits.minX;
	float height = oldLimits.maxY - oldLimits.minY;
	
	//in which direction we grow?
	if(dir.x > 0) {
		if(dir.y > 0) { // NE
			printf("NE\n");
			doubleLimits = aabb(
				oldLimits.minX, oldLimits.minY, 
				oldLimits.maxX + width, oldLimits.maxY + height);	
		} 
		else { // SE
			printf("SE\n");
			doubleLimits = aabb(
				oldLimits.minX, oldLimits.minY - height, 
				oldLimits.maxX + width, oldLimits.maxY);
		}
	} 
	else {
		if (dir.y > 0) { // NW
			printf("NW\n");			
			doubleLimits = aabb(
				oldLimits.minX - width, oldLimits.minY, 
				oldLimits.maxX, oldLimits.maxY + height);
		} 
		else { // SW
			printf("SW\n");
			doubleLimits = aabb(
				oldLimits.minX - width, oldLimits.minY - height, 
				oldLimits.maxX, oldLimits.maxY);
		}
	}

	printf("Doubled the limits to: ");
	printAABB(doubleLimits);
	QTNode *newRoot = qtNodeNew(doubleLimits);
	// i will not manually compute each quadrant, tired...
	qtNodeSplit(newRoot);
	tree->root->parent = newRoot;
	
	if(dir.x > 0) { 
		if(dir.y > 0) {// NE, 
			qtNodeDelete(newRoot->childs[SW], true);
			newRoot->childs[SW] = tree->root;
		} else {//SE
			qtNodeDelete(newRoot->childs[NW], true);
			newRoot->childs[NW] = tree->root;
		}
	} else {
		if(dir.y > 0) { //NW
			qtNodeDelete(newRoot->childs[SE], true);
			newRoot->childs[SE] = tree->root;	
		} else { // SW
			qtNodeDelete(newRoot->childs[NE], true);
			newRoot->childs[NE] = tree->root;
		}
	}
	tree->root = newRoot;
	if(!aabbFitsInAABB(newLimits, doubleLimits)) {
		quadTreeExpand(tree, newLimits);
	}
	return true;
}

QTObject *quadTreeAdd(QuadTree *tree, AABB limits, void *data) 
{
	QTObject *obj = objectNew(limits, data);
	if(!obj) {
		return NULL;
	}
	//
	if(!aabbFitsInAABB(limits, tree->root->limits)) {
		printf("quadTreeExpand is experimental\n");
		quadTreeExpand(tree, limits);
	}
	
	if(!qtNodeAdd(tree->root, obj)) {
		objectDelete(obj);
		return NULL;
	}
	obj->tree = tree;
	return obj;
}

static void printNode(QTNode *node)
{
	printf("node: {%8.2f, %8.2f, %8.2f, %8.2f}\n",
			node->limits.minX, node->limits.minY,
			node->limits.maxX, node->limits.maxY);
}


static void printNodes(QTNode *node, int depth) 
{
	int i;
	if(!node) return;

	printf("---- NodeDepth: %d\n", depth);
	printNode(node);
	printf("objects: %d\n", node->objects->len);
	for(i = 0; i < node->objects->size; i++) {	
		if(node->objects->data[i])
			printObject(node->objects->data[i]);
	}
	depth++;
	for(i = 0; i < 4; i++) {
		printNodes(node->childs[i], depth);
	}
	depth--;
}

void printTree(QuadTree *tree) 
{
	printNodes(tree->root, 0);
}

static bool qtNodeGetIntersections(QTNode *node, AABB limits, Array *result) 
{
	int i;
	// return if limits does not intersect this quad
	if(!aabbIntersect(&node->limits, &limits))  {
		return false;
	}
	// Check objects at this quad level and add them
	// if they intersect limits
	for(i = 0; i < node->objects->size; i++) {
		if(node->objects->data[i] == NULL)
			continue;
		if(aabbIntersect(&limits, &((QTObject*)node->objects->data[i])->limits)) {
			arrayPush(result, node->objects->data[i]);
		}
	}

	// return if it has no childs //
	if(node->childs[0] == NULL)
		return true;

	for(i = 0; i < QT_NUM_CHILDS; i++) {
		qtNodeGetIntersections(node->childs[i], limits, result);
	}
	return true;	
}

bool quadTreeGetIntersections(QuadTree *tree, AABB limits, Array *result) 
{
	return qtNodeGetIntersections(tree->root, limits, result);	
}


#ifdef COMPILE_TESTS

void quadTreeTest()
{
	int i;
	AABB box = aabb(-10, -10, 10, 10);
	char *str = "Surface";
	QTObject *obj;
	obj = objectNew(box, str);
	assert(obj->limits.minX == box.minX);
	assert(obj->limits.minY == box.minY);
	assert(strcmp(obj->data, str) == 0);
	objectDelete(obj);

	printf("Testing QuadTree\n");
	box = aabb(-10, -10, 10, 10);
	QuadTree *tree = quadTreeNew(box);
	assert(tree);
	assert(tree->items == 0);

	box = aabb(3,5,5,7);
	assert(quadTreeAdd(tree, box, "First Node"));
	assert(tree->root->childs[NE] == NULL);
	assert(tree->root->objects->len == 1);
	assert(((QTObject *)tree->root->objects->data[0])->limits.minX == 3);
	assert(((QTObject *)tree->root->objects->data[0])->limits.maxX == 5);

	box = aabb(-5, 2, -3, 4);
	assert(quadTreeAdd(tree, box, "Second Node"));
	assert(tree->root->childs[NE] == NULL);
	assert(tree->root->objects->len == 2);
	assert(((QTObject *)tree->root->objects->data[1])->limits.minX == -5);
	assert(((QTObject *)tree->root->objects->data[1])->limits.maxX == -3);
	assert(strcmp(((QTObject *)tree->root->objects->data[1])->data, "Second Node")== 0);

	// it should split
	box = aabb(-5, -7, -3, -5);
	assert(quadTreeAdd(tree, box, "Third Node"));


	assert(tree->root->objects->len == 0);
	assert(tree->root->childs[NE]->objects->len == 1);
	assert(tree->root->childs[NW]->objects->len == 1);
	assert(tree->root->childs[SW]->objects->len == 1);
	assert(tree->root->childs[SE]->objects->len == 0);

	box = aabb(2, 4, 4, 6);
	assert(quadTreeAdd(tree, box, "Forth Node"));


	box = aabb(1, 1, 3, 3);
	assert(quadTreeAdd(tree, box, "Fifth Node"));


	box = aabb(6, 6, 8, 8);
	assert(quadTreeAdd(tree, box, "Sixth Node"));

	Array *res = arrayNew(); 
	AABB queryBox = aabb(0, 0, 10, 10);
	quadTreeGetIntersections(tree, queryBox, res);
	assert(res->len == 4);
	for(i = 0; i < res->len; i++) {
		obj = res->data[i];
		assert(aabbIntersect(&obj->limits, &queryBox));
	}
	printf("Result 1:\n");
	for(i = 0; i < res->len; i++) {
		printObject(res->data[i]);
	}
	arrayReset(res);


	queryBox = aabb(5, 5, 10, 10);
	quadTreeGetIntersections(tree, queryBox, res);
	for(i = 0; i < res->len; i++) {
		printObject(res->data[i]);
	}
	arrayReset(res);
	arrayDelete(&res);

	//printTree(tree);
	
	quadTreeDelete(tree);

}

#endif
