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

/**
 * Object functions
 */

/**
 * Creates an object, having limits
 *
 * @param limits The boundaries of the object
 * @param data The element to store in this node
 * @return A new QTObject
 */
QTObject *objectNew(AABB limits, void *data) 
{
    QTObject *obj = malloc(sizeof(*obj));
    if (!obj) {
        perror("malloc");
        return NULL;
    }
    obj->limits = limits;
    obj->data = data;
    obj->tree = NULL;
    obj->node = NULL;

    return obj;
}

/**
 * Destroys an object
 *
 * @param obj The object to destroy
 */
void objectDelete(QTObject *obj) 
{
    free(obj);
}

/**
 * Quad Tree Nodes
 */

/**
 * Creates a new Quad Tree node
 *
 * @param limits The boundaries of the node
 *
 * @return a new QTNode
 */
static QTNode *nodeNew(AABB limits)
{
    int i;
    QTNode *node;

    node = malloc(sizeof(*node));
    if (!node) {
        perror("malloc");
        return NULL;
    }

    node->parent = NULL;
    node->limits = limits;
    node->objects = arrayNew();

    for (i = 0; i < QT_NUM_CHILDS; i++) {
        node->childs[i] = NULL;
    }

    return node;
}

/**
 * Deletes a node from the tree
 *
 * @param node The node to delete
 * @param recurse True if the childs should also be deleted
 */
static void nodeDelete(QTNode *node, bool recurse)
{
    int i;
    QTObject *obj;

    if (recurse && node->childs[NE]) {
        for (i = 0; i < QT_NUM_CHILDS; i++) {
            nodeDelete(node->childs[i], recurse);
        }
    }

    arrayForEach(node->objects, obj, i) {
        objectDelete(obj);
    }

    arrayDelete(&node->objects);
    free(node);
}

/**
 * @brief Gets the index of child where this obj fits
 *
 * Checks if this obj limits fits in one of the node childs
 * and returns the index of the child if it match one, or -1 on error
 * 
 * @param node The QTNode to check it's children
 * @param obj The QTObjects to fit
 * @return the index of child, or -1 if cannot fit in childs
 *
 */
static int nodeGetIndex(QTNode *node, QTObject *obj)
{
    int i;
    for (i = 0; i < QT_NUM_CHILDS; i++) {
        assert(node->childs[i] != NULL);
        if (aabbFitsIn(obj->limits, node->childs[i]->limits)) {
            return i;
        }
    }

    return -1;
}

/**
 * Splits the tree in 4 quadrants, and attach them
 *
 * @param node The node to split
 */
static void nodeSplit(QTNode *node)
{
    float x = node->limits.minX;
    float y = node->limits.minY;
    float maxX = node->limits.maxX;
    float maxY = node->limits.maxY;
    float halfWidth = (maxX - x) / 2.0f;
    float halfHeight = (maxY - y) / 2.0f;
    int i;

    node->childs[NE] = nodeNew(aabb(x + halfWidth, y + halfHeight, maxX, maxY));
    node->childs[NW] = nodeNew(aabb(x, y + halfHeight, x + halfWidth, maxY));
    node->childs[SW] = nodeNew(aabb(x, y, x + halfWidth, y + halfHeight));
    node->childs[SE] = nodeNew(aabb(x + halfWidth, y, maxX, y + halfHeight));

    // set parent
    for (i = 0; i < QT_NUM_CHILDS; i++) {
        node->childs[i]->parent = node;
    }
}

/**
 * Adds a new object to the node
 *
 * @param node The node to attach the object
 * @param obj The object to attach
 * @return true on success, false if object boundaries does not fit in node
 *      boundaries
 */
static bool nodeAdd(QTNode *node, QTObject *obj) 
{
    int i, idx;
    QTObject *oldObj;

    // if object cannot fit in this node //
    if (!aabbFitsIn(obj->limits, node->limits)) {
        return false;
    }

    // if is leaf and has enough objects room
    if (node->childs[NE] == NULL && node->objects->len < QT_TREE_MAX_OBJECTS) {
        arrayPush(node->objects, obj);
        obj->node = node;
        return true;
    }
    // if is leaf, not splitted and no more room
    else if (node->childs[NE] == NULL) {
        // split the node in 4
        nodeSplit(node);

        // move it's objects to childs, if possible
        arrayForEach(node->objects, oldObj, i) {
            idx = nodeGetIndex(node, oldObj);
            if (idx == -1)
                continue;
            else {
                if (nodeAdd(node->childs[idx], oldObj)) {
                    arrayUnset(node->objects, i);
                    //node->objects->data[i] = NULL;
                }
            }
        }
        arrayCompact(node->objects); // because we nulled some items
    }

    // add original node into one of the childs
    idx = nodeGetIndex(node, obj);
    if (idx == -1) {
        arrayPush(node->objects, obj);
        obj->node = node;
    } else {
        return nodeAdd(node->childs[idx], obj);
    }

    return true;
}

/**
 * Recursively climbs up the tree and remove empty nodes
 *
 * @param node The node to start to climb up
 * @return true on success, false if cannot delete any node
 */
static bool nodeDeleteUp(QTNode *node)
{	
    QTNode *n = node->parent;
    int numChildsObjs = 0; // number of objects this node children has
    int i;

    for (i = 0; i < QT_NUM_CHILDS; i++) {
        numChildsObjs += n->childs[i]->objects->len;
        if (n->childs[i]->childs[NE] != NULL || numChildsObjs > 0)
            return false;
    }
    // if childs has no objects left //
    if (numChildsObjs == 0) {
        for (i = 0; i < QT_NUM_CHILDS; i++) {
            // delete them //
            nodeDelete(n->childs[i], true);
            n->childs[i] = NULL;
        }
        // if also the node has no more objects left, climb up //
        if (n->objects->len == 0) {
            return nodeDeleteUp(n);
        }
    }

    return true;
}

/**
 * @brief Updates a tree object boundaries.
 * 
 * Updates a tree object boundaries (like when an obj changes it's coordinates)
 * by found a new position in the tree. Tree boundaries are expanded if 
 * this object boundaries does not fit enymore.
 *
 * @param obj The object to update
 * @param limit The object new boundaries
 * @return true on success
 */
bool qtObjectUpdate(QTObject *obj, AABB newLimits) 
{
    QTNode *oldNode, *curr;
    bool ret, found;
    int i;
    oldNode = obj->node;
    assert(obj);

    // if we still fits in this node bounds, do not move
    // just update the limits
    if (aabbFitsIn(newLimits, oldNode->limits)) {
        obj->limits = newLimits;
        return true;
    }
    found = false;
    // else, if we do not fit it anymore, climb up the tree
    // until we can fit inside of a node //
    for (curr = obj->node; curr; curr = curr->parent) {
        if (aabbFitsIn(newLimits, curr->limits)) {
            found = true;
            break;
        }
    }

    // probably we reached root
    if (!found) {
        quadTreeExpand(obj->tree, newLimits);
        curr = obj->tree->root;
        printf("Expaded ROOT\n");
    }

    i = arrayIndexOf(oldNode->objects, obj);
    if (i < 0) {
        fprintf(stderr, "qtObjectUpdate: cannot find the object\n");
        exit(1);
    }
    arrayUnset(oldNode->objects, i);
    arrayCompact(oldNode->objects);
    obj->limits = newLimits;

    // insert into newNode //
    ret = nodeAdd(curr, obj);
    // check if old node is empty and is leaf
    // and if it's neibghors are too, remove all
    if (oldNode->childs[NE] == 0) {
        nodeDeleteUp(oldNode);
    }

    return ret;
}

/**
 ** Public functions
 **/

QuadTree *quadTreeNew(AABB limits) 
{
    QuadTree *tree = malloc(sizeof(*tree));
    if (!tree) {
        perror("malloc");
        return NULL;
    }
    tree->root = nodeNew(limits);
    tree->items = 0;

    return tree;
}

void quadTreeDelete(QuadTree *tree) 
{
    nodeDelete(tree->root, true);
    free(tree);
}

/**
 * Expands the tree when a newLimits are bigger than the root itself limits
 *
 * @param tree The tree to expand
 * @param newLimits The new limits to expand to
 * @return true on success
 * @TODO - simplify this
 */
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

    // in which direction shall we grow?
    if (dir.x > 0) {
        if (dir.y > 0) { // NE
            printf("NE\n");
            doubleLimits = aabb(
                    oldLimits.minX,
                    oldLimits.minY,
                    oldLimits.maxX + width,
                    oldLimits.maxY + height);
        }
        else { // SE
            printf("SE\n");
            doubleLimits = aabb(
                    oldLimits.minX,
                    oldLimits.minY - height,
                    oldLimits.maxX + width,
                    oldLimits.maxY);
        }
    }
    else {
        if (dir.y > 0) { // NW
            printf("NW\n");
            doubleLimits = aabb(
                    oldLimits.minX - width,
                    oldLimits.minY,
                    oldLimits.maxX,
                    oldLimits.maxY + height);
        } 
        else { // SW
            printf("SW\n");
            doubleLimits = aabb(
                    oldLimits.minX - width,
                    oldLimits.minY - height,
                    oldLimits.maxX,
                    oldLimits.maxY);
        }
    }

    printf("Doubled the limits to: ");
    printAABB(doubleLimits);
    QTNode *newRoot = nodeNew(doubleLimits);
    // i will not manually compute each quadrant...
    nodeSplit(newRoot);
    tree->root->parent = newRoot;

    if (dir.x > 0) {
        if (dir.y > 0) { // NE,
            nodeDelete(newRoot->childs[SW], true);
            newRoot->childs[SW] = tree->root;
        } else { //SE
            nodeDelete(newRoot->childs[NW], true);
            newRoot->childs[NW] = tree->root;
        }
    } else {
        if (dir.y > 0) { //NW
            nodeDelete(newRoot->childs[SE], true);
            newRoot->childs[SE] = tree->root;
        } else { // SW
            nodeDelete(newRoot->childs[NE], true);
            newRoot->childs[NE] = tree->root;
        }
    }

    tree->root = newRoot;
    // if newLimits still does not fit in the doubled ones, recurse
    if (!aabbFitsIn(newLimits, doubleLimits)) {
        quadTreeExpand(tree, newLimits);
    }

    return true;
}

QTObject *quadTreeAdd(QuadTree *tree, AABB limits, void *data) 
{
    QTObject *obj = objectNew(limits, data);
    if (!obj) {
        return NULL;
    }

    if (!aabbFitsIn(limits, tree->root->limits)) {
        printf("quadTreeExpand is experimental\n");
        quadTreeExpand(tree, limits);
    }

    if (!nodeAdd(tree->root, obj)) {
        objectDelete(obj);
        return NULL;
    }
    obj->tree = tree;

    return obj;
}

/**
 * @brief Query the tree for objects that intersects this limits
 *
 * Gets all the objects from this node down that intersects 
 * the given AABB limits, and return them in result array.
 * 
 * @param node The start node to check for intersections down
 * @param limits The AABB thease objects intersects with
 * @param result The Array where to put the results
 * @return false if this node does not intersect with the limits
 *
 */
static bool nodeGetIntersections(QTNode *node, AABB limits, Array *result) 
{
    int i;
    QTObject *obj;

    // return if limits does not intersect this quad
    if (!aabbIntersects(&node->limits, &limits))  {
        return false;
    }
    // Check objects at this quad level and add them if they intersect
    arrayForEach(node->objects, obj, i) {
        if (aabbIntersects(&limits, &obj->limits)) {
            arrayPush(result, obj);
        }
    }

    // return if it has no childs //
    if (node->childs[0] == NULL)
        return true;

    for (i = 0; i < QT_NUM_CHILDS; i++) {
        nodeGetIntersections(node->childs[i], limits, result);
    }

    return true;
}

bool quadTreeGetIntersections(QuadTree *tree, AABB limits, Array *result) 
{
    return nodeGetIntersections(tree->root, limits, result);
}

#ifdef COMPILE_TESTS

static void printObject(QTObject *obj)
{
    printf("surf: {%8.2f, %8.2f, %8.2f, %8.2f}, %p\n",
            obj->limits.minX, obj->limits.minY,
            obj->limits.maxX, obj->limits.maxY,
            obj->data);
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

    if (!node) return;

    printf("---- NodeDepth: %d\n", depth);
    printNode(node);
    printf("objects: %d\n", node->objects->len);

    for (i = 0; i < node->objects->size; i++) {
        if (node->objects->data[i])
            printObject(node->objects->data[i]);
    }
    depth++;
    for (i = 0; i < 4; i++) {
        printNodes(node->childs[i], depth);
    }
    depth--;
}

void printTree(QuadTree *tree) 
{
    printNodes(tree->root, 0);
}

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
    for (i = 0; i < res->len; i++) {
        obj = res->data[i];
        assert(aabbIntersects(&obj->limits, &queryBox));
    }

    //printf("Result 1:\n");
    for (i = 0; i < res->len; i++) {
        //	printObject(res->data[i]);
    }
    arrayReset(res);


    queryBox = aabb(5, 5, 10, 10);
    quadTreeGetIntersections(tree, queryBox, res);
    for (i = 0; i < res->len; i++) {
        //	printObject(res->data[i]);
    }
    arrayReset(res);
    arrayDelete(&res);

    //printTree(tree);

    quadTreeDelete(tree);
}

#endif // COMPILE_TESTS

