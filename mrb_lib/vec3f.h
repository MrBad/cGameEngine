/**
 * 3D float vectors
 */
#ifndef VEC3F_H
#define VEC3F_H

#include <math.h>

typedef struct {
    float x;
    float y;
    float z;
} Vec3f;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static inline Vec3f vec3f(float x, float y, float z) 
{
    return (Vec3f) { x, y, z };
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
    if (len > 0) {
        return (Vec3f) { vec.x / len, vec.y / len, vec.z / len };
    }

    return (Vec3f) { 0.0f, 0.0f, 0.0f };
}

typedef enum { X_AXIS, Y_AXIS, Z_AXIS } AXIS;
static inline Vec3f vec3fRotate(Vec3f vec, float angleDegrees, AXIS axis) 
{
    float radians = angleDegrees * M_PI / 180;
    float x = vec.x, y = vec.y, z = vec.z;
    float csn = cosf(radians);
    float sn = sinf(radians);

    switch (axis) {
        case X_AXIS:
            return (Vec3f) { x, y * csn - z * sn, y * sn + z * csn };
        case Y_AXIS:
            return (Vec3f) { x * csn - y * sn, y, -x * sn + z * csn };
        case Z_AXIS:
        default:
            return (Vec3f) { x * csn - y * sn, x * sn + y * csn, z};
    }
}

static inline Vec3f vec3fAdd(Vec3f a, Vec3f b) 
{
    return (Vec3f) { a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline Vec3f vec3fAddS(Vec3f vec, float scalar) 
{
    return (Vec3f) { vec.x + scalar, vec.y + scalar, vec.z + scalar };
}

static inline Vec3f vec3fSub(Vec3f a, Vec3f b) 
{
    return (Vec3f) { a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline Vec3f vec3fSubS(Vec3f vec, float scalar) 
{
    return (Vec3f) { vec.x - scalar, vec.y - scalar, vec.z - scalar };
}

static inline Vec3f vec3fMul(Vec3f a, Vec3f b) 
{
    return (Vec3f) { a.x * b.x, a.y * b.y, a.z * b.z };
}

static inline Vec3f vec3fMulS(Vec3f vec, float scalar) 
{
    return (Vec3f) { vec.x * scalar, vec.y * scalar, vec.z * scalar };
}

static inline Vec3f vec3fDiv(Vec3f a, Vec3f b) 
{
    return (Vec3f) { a.x / b.x, a.y / b.y, a.z / b.z };
}

static inline Vec3f vec3fDivS(Vec3f vec, float scalar) 
{
    return (Vec3f) { vec.x / scalar, vec.y / scalar, vec.z / scalar };
}

#endif

