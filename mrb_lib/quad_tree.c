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
QTSurface *surfaceNew(AABB limits, void *data) 
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
void surfaceDelete(QTSurface *surface) 
{
	free(surface);
}


static void printSurface(QTSurface *s)
{
	printf("surf: {%8.2f, %8.2f, %8.2f, %8.2f}, %p\n",
			s->limits.minX, s->limits.minY,
			s->limits.maxX, s->limits.maxY,
			s->data);
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
	fprintf(stdout, "surfacesAdd - probably memory corruption\n");
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
	node->parent = NULL;
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

static bool qtNodeAdd(QTNode *node, QTSurface *surface) 
{
	int i, idx;
	// object cannot fit in this node //
	if(!aabbFitsInAABB(surface->limits, node->limits)) {
		fprintf(stderr, "---\nSurface out of bounds\n");
		printSurface(surface);
		fprintf(stderr, "Write that tree expand functions\n");
		return false;
	}
	// if is leaf and has enough objects room
	if(node->childs[NE] == NULL && node->surfaces->items < QT_TREE_MAX_OBJECTS) {
		surfacesAdd(node->surfaces, surface);
		surface->node = node;
		return true;
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
		surface->node = node;
	} else {
		return qtNodeAdd(node->childs[idx], surface);
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
		surfacesLeft += n->childs[i]->surfaces->items;
		if(n->childs[i]->childs[NE] != NULL || surfacesLeft > 0)
			return false;
	}
	if(surfacesLeft == 0) {
		for(i = 0; i < QT_NUM_CHILDS; i++) {
			qtNodeDelete(n->childs[i], true);
			n->childs[i] = NULL;
		}
		if(n->surfaces->items == 0) {
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

bool surfaceUpdate(QTSurface *surface, AABB newLimits) 
{

	QTNode *oldNode, *curr;
	bool ret, found;
	int i;
	oldNode = surface->node;
	assert(surface);	
	// if we still fits in this node bounds, do not move
	// just update the limits
	if(aabbFitsInAABB(newLimits, oldNode->limits)) {
		surface->limits = newLimits;
		return true;
	}
	found = false;
	// else, if we do not fit it anymore, climb up the tree
	// until we can fit inside of a node //
	for(curr = surface->node; curr; curr = curr->parent) {
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
		quadTreeExpand(surface->tree, newLimits);
		////////////////////////
		assert(curr != NULL);
		printAABB(newLimits);
		printAABB(surface->node->limits);
		//exit(1);
		//return false;
	}
	
	found = false;
	// remove the reference from oldNode list //
	for(i = 0; i < oldNode->surfaces->size; i++) {
		if(!oldNode->surfaces->data[i]) continue;
		if(oldNode->surfaces->data[i] == surface) {
			oldNode->surfaces->data[i] = NULL;
			oldNode->surfaces->items--;
			found = true;
			break;
		}	
	}
	if(!found) {
		fprintf(stderr, "We did not found the surface\n");
		return false;
	}
	surface->limits = newLimits;

	// insert into newNode //
	ret = qtNodeAdd(curr, surface);
	
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

QTSurface *quadTreeAdd(QuadTree *tree, AABB limits, void *data) 
{
	QTSurface *surface = surfaceNew(limits, data);
	if(!surface) {
		return NULL;
	}
	//
	if(!aabbFitsInAABB(limits, tree->root->limits)) {
		printf("quadTreeExpand is experimental\n");
		quadTreeExpand(tree, limits);
	}
	
	if(!qtNodeAdd(tree->root, surface)) {
		surfaceDelete(surface);
		return NULL;
	}
	surface->tree = tree;
	return surface;
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

void printTree(QuadTree *tree) 
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

void quadTreeDeleteResults(QTSurfaces *results) 
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
	quadTreeDeleteResults(res);

	//printTree(tree);
	
	quadTreeDelete(tree);

}

#endif
