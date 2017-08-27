#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "aabb.h"
#include "list.h"

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
