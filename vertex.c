#include "vertex.h"

int vertexSetPos(Vertex *v, float x, float y) 
{
	v->pos.x = x;
	v->pos.y = y;
	return 0;
}


int vertexSetColor(Vertex *v, float r, float g, float b, float a) 
{
	v->color.r = r;
	v->color.g = g;
	v->color.b = b;
	v->color.a = a;
	return 0;
}

int vertexSetUV(Vertex *vert, float u, float v) 
{
	vert->uv.u = u;
	vert->uv.v = v;
	return 0;
}
