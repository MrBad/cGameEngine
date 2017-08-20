#include "mat4f.h"


Mat4f mat4fOrtho(float left, float right, float top, float bottom, float zNear, float zFar)
{
    float l = left, r = right, t = top, b = bottom, n = zNear, f = zFar;
    float tx = -(r + l) / (r - l);
    float ty = -(t + b) / (t - b);
	float tz;
	if(f+n == 0)
		tz = 0;
	else
		tz = -(f + n) / (f - n);

	return mat4f(
         2 / (r - l),  0,            0,            tx,
         0,            2 / (t - b),  0,            ty,
         0,            0,            -2 / (f - n), tz,
         0,            0,            0,            1
    );
}


void mat4fPrint(Mat4f mat) {
    int i;
    for(i = 0; i < 4; i++)
            printf("|0%d:%12f 1%d:%12f 2%d:%12f 3%d%12f| \n",
                    i, mat.m[0][i], i, mat.m[1][i], i, mat.m[2][i], i, mat.m[3][i]);
}

