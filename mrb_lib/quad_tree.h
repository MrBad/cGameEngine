#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "aabb.h"
#include "list.h"


//
//	TODO - add an update node function that will search for old node
//		and if changed, remove it and reinsert it back with new 
//		positions - this will help me not rebuild all the tree
//		every frame and speed up things. Because sprites will not 
//		change their position too much, it meens they don't need to be moved
//		far into the tree - so a good algorithm will be to keep track of 
//		parent nodes and walk up the tree until it fits. 
//		On reinsert - just go up into the tree from current node
//		and check nearest planes... this will be faster than reinserting sprite
//		into root...
//		Maybe QTSurface is not a good name - maybe i should call it QTEntity
//		and QTEntities
//		Another optimisation will be to keep a reference between User and Entity
//		so we can fast update the node
//		- add an grow function - that will grow the tree up if 
//		an new node with bigger limits than root try to be inserted
//		This way we can have an infinite map...
//		- write more tests to check for memory leaks / valgrind check

enum {
	NE,	// first quadran
	NW, // second
	SW,	// third
	SE,	// forth
	QT_NUM_CHILDS
};
 
typedef struct QTNode QTNode;

// an object in the node //
typedef struct {
	AABB limits;	// this object bounding box
	QTNode *node;		// node where it is insert
	void *data;		// pointer to whatever aditional info needed
} QTSurface;


typedef struct {
	QTSurface **data;		// ptr to ptr of surfaces
	int items;				// number of surfaces in array
	int size;				// size of array in elements
} QTSurfaces;

// a node
struct QTNode {
	struct QTNode *parent;		// parent node to whom it belongs this node
	struct QTNode *childs[4];	// this node childs 0 - NE, 1 - NW, 2 - SW, 3 - SE
	AABB limits;				// boundary box this node represents
	QTSurfaces *surfaces;		// array of pointers containing surfaces in this node
};

typedef struct {
	QTNode *root;
	int items;
} QuadTree;


// surface - maybe i will call it Entity //
QTSurface *surfaceNew(AABB limits, void *data);
void surfaceDelete(QTSurface *surface);

bool QTSurfaceUpdate(QTSurface *surface, AABB newLimits);

QuadTree *quadTreeNew(AABB limits);
void quadTreeDelete(QuadTree *tree);
QTSurface *quadTreeAdd(QuadTree *tree, AABB limits, void *data);
//bool quadTreeAddSurface(QuadTree *tree, QTSurface *surface);

QTSurfaces *surfacesNew();
// query
bool quadTreeGetIntersections(QuadTree *tree, AABB limits, QTSurfaces *results);
void quadTreeResetResults(QTSurfaces *results);
void quadTreeDeleteResults(QTSurfaces *results); 

void quadTreeTest();


#endif
