/**
 * A Quad Tree implementation
 * Docs: https://en.wikipedia.org/wiki/Quadtree
 * TODO - write more tests. Check for memody leaks
 */
#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "aabb.h"
#include "array.h"

/* Child nodes types */
enum {
    NE, // first quadran
    NW, // second
    SW, // third
    SE, // forth
    QT_NUM_CHILDS
};

/**
 * How many objects should we keep in a node before splitting it up
 */
#define QT_TREE_MAX_OBJECTS 2

typedef struct QTNode QTNode;
typedef struct QuadTree QuadTree;

/* An object in the node */
typedef struct {
    AABB limits;        // this object bounding box
    QuadTree *tree;     // a reference to tree, so i can expand it if i need
    QTNode *node;       // node where it belongs to
    void *data;         // pointer to user element/object/data
    int type;           // type of object pointing to
} QTObject;

/* A node in the tree */
struct QTNode {
    struct QTNode *parent;      // parent node to whom it belongs this node
    struct QTNode *childs[4];   // this node childs 0: NE, 1: NW, 2: SW, 3: SE
    AABB limits;                // boundary box this node represents
    Array *objects;     // array of pointers containing objects in this node
};

/* A quad tree */
struct QuadTree {
    QTNode *root;
    int items;
};

/**
 * Updates the position of the object in the tree
 *
 * @param obj The QTObject to update
 * @param newLimits The new AABB position
 *
 * @return true on success
 */
bool qtObjectUpdate(QTObject *obj, AABB newLimits);

/**
 * Creates a new quad tree, with the given limits
 *
 * @param limits The quad tree limits
 * @return a reference to this tree
 */
QuadTree *quadTreeNew(AABB limits);

/**
 * Destroys the tree
 *
 * @param tree The tree to destroy
 */
void quadTreeDelete(QuadTree *tree);

/**
 * Adds an element to the tree
 *
 * @param tree The tree to add the element to
 * @param limits The element limits
 * @param element The element to add (data)
 */
QTObject *quadTreeAdd(QuadTree *tree, AABB limits, void *element);

// TODO quadTreeDel tree, QTObject, based on nodeDelete.

/**
 * Query the tree for the objects that intersects this limits
 * and put them into results Array
 *
 * @param tree Tree to query
 * @param limits AABB square to query for
 * @param results Array of QTObject where to store results
 */
bool quadTreeGetIntersections(QuadTree *tree, AABB limits, Array *results);

/**
 * Resets the results array (keeps the so far allocated memory, sets items to 0)
 *
 * @param results The array to reset
 */
void quadTreeResetResults(Array *results);

/**
 * Deletes the results array
 */
void quadTreeDeleteResults(Array *results); 

int quadTreemaxDepth(QTNode *node);
/**
 * Do some tests
 */
void quadTreeTest();

#endif // QUAD_TREE_H

