#include "quad_tree.h"
#include "aabb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string.h>

// Maximum number of objects in a node 
// before splitting node //
#define QT_TREE_MAX_OBJECTS 2

//
// Surface functions //
//
// creates a surface
static QTSurface *surfaceNew(AABB limits, void *data) 
{
	QTSurface *s = malloc(sizeof(QTSurface));
	if(!s) {
		perror("malloc");
		return NULL;
	}

	s->limits = limits;
	s->data = data;
	return s;
}


// destroy surface
static void surfaceDelete(QTSurface *surface) 
{
	free(surface);
}
//
// Array of surfaces //
//

QTSurfaces *surfacesNew() 
{
	QTSurfaces *s = malloc(sizeof(*s));
	if(!s) {
		perror("malloc");
		return NULL;
	}
	s->data = NULL;
	s->items = 0;
	s->size = 0;
	return s;
}

static void surfacesDelete(QTSurfaces *surfaces) 
{
	int i;
	assert(surfaces);

	if(surfaces->data) {
		for(i = 0; i < surfaces->size; i++) {
			if(surfaces->data[i]) {
				surfaceDelete(surfaces->data[i]);
			}
		}
	}
	free(surfaces->data);
	free(surfaces);	
}


static bool surfacesGrow(QTSurfaces *arr) 
{

	int numElements = arr->size == 0 ? 16 : arr->size * 2;
	arr->data = realloc(arr->data, numElements * sizeof(*arr->data));
	if(!arr->data) {
		perror("realloc");
		return false;
	}
	while(arr->size < numElements)
		arr->data[arr->size++] = NULL;

	return true;
}

static bool surfacesAdd(QTSurfaces *arr, QTSurface *s) 
{

	if(arr->items == arr->size) {
		if(!surfacesGrow(arr)) {
			return false;
		}
	}
	for(int i = 0; i < arr->size; i++) {
		if(arr->data[i] == NULL) {
			arr->data[i] = s;
			arr->items++;
			return true;
		}
	}
	fprintf(stdout, "WTF\n");
	exit(1);
	return false;
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
	node->limits = limits;
	node->surfaces = surfacesNew();

	for(i = 0; i < QT_NUM_CHILDS; i++) {
		node->childs[i] = NULL;
	}

	return node;
}


static void qtNodeDelete(QTNode *node, bool recurse)
{
	int i;
	if(recurse && node->childs[NE]) {
		for(i = 0; i < QT_NUM_CHILDS; i++) {
			qtNodeDelete(node->childs[i], recurse);
		}
	}
	if(node->surfaces)
		surfacesDelete(node->surfaces);

	free(node);
}


// check if this obj fits into one of this node childs
// and return an index of which child belongs, or -1
// if does not fit
static int qtNodeGetIndex(QTNode *node, QTSurface *surface)
{
	int i;
	for(i = 0; i < QT_NUM_CHILDS; i++) {
		assert(node->childs[i] != NULL);
		if(aabbFitsInAABB(surface->limits, node->childs[i]->limits))
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

	node->childs[NE] = qtNodeNew(aabb(x + halfWidth, y + halfHeight, maxX, maxY));
	node->childs[NW] = qtNodeNew(aabb(x, y + halfHeight, x + halfWidth, maxY));
	node->childs[SW] = qtNodeNew(aabb(x, y, x + halfWidth, y + halfHeight));
	node->childs[SE] = qtNodeNew(aabb(x + halfWidth, y, maxX, y + halfHeight));
}

static bool qtNodeAdd(QTNode *node, QTSurface *surface) 
{
	int i, idx;
	// object cannot fit in this node //
	if(!aabbFitsInAABB(surface->limits, node->limits)) {
		return false;
	}
	// if is leaf and has enough objects room
	if(node->childs[NE] == NULL && node->surfaces->items < QT_TREE_MAX_OBJECTS) {
		return surfacesAdd(node->surfaces, surface);
	}
	// if is leaf, not splitted and no more room
	else if(node->childs[NE] == NULL) {
		qtNodeSplit(node);
		for(i = 0; i < node->surfaces->size; i++) {
			if(!node->surfaces->data[i]) {
				continue;
			}
			QTSurface *oldSurface = node->surfaces->data[i];
			idx = qtNodeGetIndex(node, oldSurface);
			if(idx == -1) {
				continue;
			} else {
				if(qtNodeAdd(node->childs[idx], oldSurface)) {
					node->surfaces->data[i] = NULL;
					node->surfaces->items--;
				}
			}
		}
	}
	// add original node into one of the childs
	idx = qtNodeGetIndex(node, surface);
	if(idx == -1) {
		surfacesAdd(node->surfaces, surface);
	} else {
		return qtNodeAdd(node->childs[idx], surface);
	}
	return true;
}


//
// Public functions
//

// All operations are done on Quad Tree
QuadTree *quadTreeNew(AABB limits) {
	QuadTree *tree = malloc(sizeof(*tree));
	if(!tree) {
		perror("malloc");
		return NULL;
	}
	tree->root = qtNodeNew(limits);
	tree->items = 0;
	return tree;
}

void quadTreeDelete(QuadTree *tree) {
	qtNodeDelete(tree->root, true);
	free(tree);
}

bool quadTreeAdd(QuadTree *tree, AABB limits, void *data) 
{
	QTSurface *surface = surfaceNew(limits, data);
	if(!surface) {
		return false;
	}
	if(!qtNodeAdd(tree->root, surface)) {
		surfaceDelete(surface);
		return false;
	}
	return true;
}

static void printNode(QTNode *node)
{
	printf("node: {%8.2f, %8.2f, %8.2f, %8.2f}\n",
			node->limits.minX, node->limits.minY,
			node->limits.maxX, node->limits.maxY);
}

static void printSurface(QTSurface *s)
{
	printf("surf: {%8.2f, %8.2f, %8.2f, %8.2f}, %s\n",
			s->limits.minX, s->limits.minY,
			s->limits.maxX, s->limits.maxY,
			(char *)s->data);
}

static void printNodes(QTNode *node, int depth) 
{
	int i;
	if(!node) return;

	printf("---- NodeDepth: %d\n", depth);
	printNode(node);
	printf("objects: %d\n", node->surfaces->items);
	for(i = 0; i < node->surfaces->size; i++) {	
		if(node->surfaces->data[i])
			printSurface(node->surfaces->data[i]);
	}
	depth++;
	for(i = 0; i < 4; i++) {
		printNodes(node->childs[i], depth);
	}
	depth--;
}
static void printTree(QuadTree *tree) 
{
	printNodes(tree->root, 0);
}

static bool qtNodeGetIntersections(QTNode *node, AABB limits, QTSurfaces *result) 
{
	int i;
	// return if limits does not intersect this quad
	if(!aabbIntersect(&node->limits, &limits))  {
		return false;
	}
	// Check objects at this quad level and add them
	// if they intersect limits
	for(i = 0; i < node->surfaces->size; i++) {
		if(node->surfaces->data[i] == NULL)
			continue;
		if(aabbIntersect(&limits, &node->surfaces->data[i]->limits)) {
			surfacesAdd(result, node->surfaces->data[i]);
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

bool quadTreeGetIntersections(QuadTree *tree, AABB limits, QTSurfaces *result) 
{
	return qtNodeGetIntersections(tree->root, limits, result);	
}

void quadTreeResetResults(QTSurfaces *results) 
{
	results->items = 0;
	memset(results->data, 0, results->size * sizeof(*results->data));
}

void quadTreeFreeResults(QTSurfaces *results) 
{
	if(results) 
		surfacesDelete(results);
}


#ifdef COMPILE_TESTS

void quadTreeTest()
{
	int i;
	AABB box = aabb(-10, -10, 10, 10);
	char *str = "Surface";
	QTSurface *s;
	s = surfaceNew(box, str);
	assert(s->limits.minX == box.minX);
	assert(s->limits.minY == box.minY);
	assert(strcmp(s->data, str) == 0);
	surfaceDelete(s);

	printf("Testing QuadTree\n");
	box = aabb(-10, -10, 10, 10);
	QuadTree *tree = quadTreeNew(box);
	assert(tree);
	assert(tree->items == 0);

	box = aabb(3,5,5,7);
	assert(quadTreeAdd(tree, box, "First Node"));
	assert(tree->root->childs[NE] == NULL);
	assert(tree->root->surfaces->items == 1);
	assert(((QTSurface *)tree->root->surfaces->data[0])->limits.minX == 3);
	assert(((QTSurface *)tree->root->surfaces->data[0])->limits.maxX == 5);

	box = aabb(-5, 2, -3, 4);
	assert(quadTreeAdd(tree, box, "Second Node"));
	assert(tree->root->childs[NE] == NULL);
	assert(tree->root->surfaces->items == 2);
	assert(((QTSurface *)tree->root->surfaces->data[1])->limits.minX == -5);
	assert(((QTSurface *)tree->root->surfaces->data[1])->limits.maxX == -3);
	assert(strcmp(((QTSurface *)tree->root->surfaces->data[1])->data, "Second Node")== 0);

	// it should split
	box = aabb(-5, -7, -3, -5);
	assert(quadTreeAdd(tree, box, "Third Node"));


	assert(tree->root->surfaces->items == 0);
	assert(tree->root->childs[NE]->surfaces->items == 1);
	assert(tree->root->childs[NW]->surfaces->items == 1);
	assert(tree->root->childs[SW]->surfaces->items == 1);
	assert(tree->root->childs[SE]->surfaces->items == 0);

	box = aabb(2, 4, 4, 6);
	assert(quadTreeAdd(tree, box, "Forth Node"));


	box = aabb(1, 1, 3, 3);
	assert(quadTreeAdd(tree, box, "Fifth Node"));


	box = aabb(6, 6, 8, 8);
	assert(quadTreeAdd(tree, box, "Sixth Node"));

	QTSurfaces *res = surfacesNew(); 
	AABB queryBox = aabb(0, 0, 10, 10);
	quadTreeGetIntersections(tree, queryBox, res);
	assert(res->items == 4);
	for(i = 0; i < res->items; i++) {
		s = res->data[i];
		assert(aabbIntersect(&s->limits, &queryBox));
	}
	printf("Result 1:\n");
	for(i = 0; i < res->items; i++) {
		printSurface(res->data[i]);
	}
	quadTreeResetResults(res);


	queryBox = aabb(5, 5, 10, 10);
	quadTreeGetIntersections(tree, queryBox, res);
	for(i = 0; i < res->items; i++) {
		printSurface(res->data[i]);
	}
	quadTreeResetResults(res);
	quadTreeFreeResults(res);

	printTree(tree);
	
	quadTreeDelete(tree);

}

#endif
