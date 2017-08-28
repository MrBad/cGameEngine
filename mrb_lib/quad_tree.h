#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "aabb.h"
#include "list.h"


//
//	TODO - add an update node function that will search for old node
//		and if changed, remove it and reinsert it back with new 
//		positions - this will help me not rebuild all the tree
//		every frame and speed up things
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

// an object in the node //
typedef struct {
	AABB limits;	// this object bounding box
	void *data;		// pointer to whatever aditional info needed
} QTSurface;


typedef struct {
	QTSurface **data;		// ptr to ptr of surfaces
	int items;				// number of surfaces in array
	int size;				// size of array in elements
} QTSurfaces;

// a node
typedef struct QTNode {
	struct QTNode *childs[4];
	AABB limits;			// boundary box this node represents
	QTSurfaces *surfaces;	// array of pointers containing surfaces in this node
} QTNode;

typedef struct {
	QTNode *root;
	int items;
} QuadTree;



QuadTree *quadTreeNew(AABB limits);
void quadTreeDelete(QuadTree *tree);
bool quadTreeAdd(QuadTree *tree, AABB limits, void *data);

QTSurfaces *surfacesNew();
bool quadTreeGetIntersections(QuadTree *tree, AABB limits, QTSurfaces *results);
void quadTreeResetResults(QTSurfaces *results);
void quadTreeFreeResults(QTSurfaces *results); 

void quadTreeTest();


#endif
