#ifndef MAT4F_H
#define MAT4F_H
#include <stdio.h>
#include "vec3f.h"

//typedef float Mat4f[4][4];

typedef union {
	float m[4][4];
	struct {
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	} indv;
} Mat4f;


// stored for opengl in format:
// # 00 10 20 30
// # 01 11 21 31
// # 02 12 22 32
// # 03 13 23 33


static inline Mat4f mat4f(
	float m00, float m10, float m20, float m30,
	float m01, float m11, float m21, float m31,
	float m02, float m12, float m22, float m32,
	float m03, float m13, float m23, float m33
		) {

	return (Mat4f) {
		.m[0][0] = m00, .m[1][0] = m10, .m[2][0] = m20, .m[3][0] = m30,
		.m[0][1] = m01, .m[1][1] = m11, .m[2][1] = m21, .m[3][1] = m31,
		.m[0][2] = m02, .m[1][2] = m12, .m[2][2] = m22, .m[3][2] = m32,
		.m[0][3] = m03, .m[1][3] = m13, .m[2][3] = m23, .m[3][3] = m33
	};
}

static inline Mat4f mat4fIdentity() 
{
	return mat4f(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1		
	);
}

static inline Mat4f mat4fTranslate(Vec3f offset) 
{
	return mat4f(
		1, 0, 0, offset.x,
		0 ,1, 0, offset.y,
		0, 0, 1, offset.z,
		0, 0, 0, 1		
	);
}

static inline Mat4f mat4fScale(Vec3f scale) 
{
	float x = scale.x, y=scale.y, z=scale.z;
	return mat4f(
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1		
	);
}


Mat4f mat4fOrtho(float left, float right, float top, float bottom, float back, float front);


static inline Mat4f mat4fMul(Mat4f a, Mat4f b) 
{
	Mat4f res = mat4fIdentity();
	int i, j, k;
	
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			float sum = 0;
			for(k = 0; k < 4; k ++) {
				sum += a.m[k][j] * b.m[i][k];
			}
			res.m[i][j] = sum;
		}
	}
	return res;
}

void mat4fPrint(Mat4f mat);
#endif
