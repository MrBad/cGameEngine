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
	AABB limits;		// this object bounding box
	void *data;		// holds aditional data about item
} QuadTreeObj;


QuadTreeObj *quadTreeObjNew(AABB limits, void *data);
void quadTreeObjDelete(QuadTreeObj *obj);

// a node
typedef struct QuadTree {
	struct QuadTree *childs[4];
	AABB box;			// boundary box
	List *objects;
} QuadTree;

QuadTree *quadTreeNew();

void quadTreeTest();


#endif
