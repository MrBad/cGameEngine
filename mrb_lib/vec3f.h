#ifndef VEC3F_H
#define VEC3F_H

#include <math.h>

// 3D float vectors 


typedef struct {
	float x;
   	float y;
	float z;
} Vec3f;

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static inline Vec3f vec3f(float x, float y, float z) 
{
	return (Vec3f) {x, y, z};
}

static inline float vec3fLength(Vec3f vec) 
{
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

static inline float vec3fDot(Vec3f a, Vec3f b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3f vec3fNormalize(Vec3f vec) 
{
	float len = vec3fLength(vec);
	if(len > 0) {
		return (Vec3f) { vec.x/len, vec.y/len, vec.z/len };
	}
	return (Vec3f) {0.0f, 0.0f, 0.0f};
}

/*
static inline Vec2f vec3fRotate(Vec2f vec, float angleDegrees) 
{
	float radians = angleDegrees * M_PI / 180;
	float r_cos = cosf(radians);
	float r_sin = sinf(radians);
	return (Vec2f) { 
		vec.x * r_cos - vec.y * r_sin,
		vec.x * r_sin + vec.y * r_cos
	};
}
*/

static inline Vec3f vec3fAdd(Vec3f a, Vec3f b) 
{
	return (Vec3f) {a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3f vec3fAddS(Vec3f vec, float scalar) 
{
	return (Vec3f) {vec.x + scalar, vec.y + scalar, vec.z + scalar};
}

static inline Vec3f vec3fSub(Vec3f a, Vec3f b) 
{
	return (Vec3f) {a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3f vec3fSubS(Vec3f vec, float scalar) 
{
	return (Vec3f) {vec.x - scalar, vec.y - scalar, vec.z - scalar};
}

static inline Vec3f vec3fMul(Vec3f a, Vec3f b) 
{
	return (Vec3f) {a.x * b.x, a.y * b.y, a.z * b.z};
}

static inline Vec3f vec3fMulS(Vec3f vec, float scalar) 
{
	return (Vec3f) {vec.x * scalar, vec.y * scalar, vec.z * scalar};
}

static inline Vec3f vec3fDiv(Vec3f a, Vec3f b) 
{
	return (Vec3f) {a.x / b.x, a.y / b.y, a.z / b.z};	
}

static inline Vec3f vec3fDivS(Vec3f vec, float scalar) 
{
	return (Vec3f) {vec.x / scalar, vec.y / scalar, vec.z / scalar};
}

#endif
