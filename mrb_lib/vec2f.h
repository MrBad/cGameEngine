#ifndef VEC2F_H
#define VEC2F_H

#include <math.h>

// 2D float vectors 

typedef struct {
	float x;
   	float y;
} Vec2f;

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static inline Vec2f vec2f(float x, float y) 
{
	return (Vec2f) {x, y};
}

static inline float vec2fLength(Vec2f vec) 
{
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}
static inline float vec2fSquaredLength(Vec2f vec) 
{
	return vec.x * vec.x + vec.y * vec.y;
}
static inline float vec2fDot(Vec2f a, Vec2f b) 
{
	return a.x * b.x + a.y * b.y;
}

static inline Vec2f vec2fNormalize(Vec2f vec) 
{
	float len = vec2fLength(vec);
	if(len > 0) {
		return (Vec2f) { vec.x/len, vec.y/len };
	}
	return (Vec2f) {0.0f, 0.0f};
}

static inline Vec2f vec2fRotate(Vec2f vec, float angleDegrees) 
{
	float radians = angleDegrees * M_PI / 180;
	float r_cos = cosf(radians);
	float r_sin = sinf(radians);
	return (Vec2f) { 
		vec.x * r_cos - vec.y * r_sin,
		vec.x * r_sin + vec.y * r_cos
	};
}

static inline Vec2f vec2fAdd(Vec2f a, Vec2f b) 
{
	return (Vec2f) {a.x + b.x, a.y + b.y};
}

static inline Vec2f vec2fAddS(Vec2f vec, float scalar) 
{
	return (Vec2f) {vec.x + scalar, vec.y + scalar};
}

static inline Vec2f vec2fSub(Vec2f a, Vec2f b) 
{
	return (Vec2f) {a.x - b.x, a.y - b.y};
}

static inline Vec2f vec2fSubS(Vec2f vec, float scalar) 
{
	return (Vec2f) {vec.x - scalar, vec.y - scalar};
}

static inline Vec2f vec2fMul(Vec2f a, Vec2f b) 
{
	return (Vec2f) {a.x * b.x, a.y * b.y};
}

static inline Vec2f vec2fMulS(Vec2f vec, float scalar) 
{
	return (Vec2f) {vec.x * scalar, vec.y * scalar};
}

static inline Vec2f vec2fDiv(Vec2f a, Vec2f b) 
{
	return (Vec2f) {a.x / b.x, a.y / b.y};	
}

static inline Vec2f vec2fDivS(Vec2f vec, float scalar) 
{
	return (Vec2f) {vec.x / scalar, vec.y / scalar};
}

#endif
