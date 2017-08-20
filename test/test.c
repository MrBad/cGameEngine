#include <stdio.h>
#include "../math_lib/vec2f.h"
#include "../math_lib/mat4f.h"


int main()
{
	Vec2f vector = vec2f(3, 4);//{3.0f, 4.0f};
	Vec2f normal = vec2f(0, 0);
	printf("Length: %f\n", vec2fLength(vector));
	
	normal = vec2fNormalize(vector);

	printf("vec: %f, %f, len: %f\nnorm: %f, %f, len:%f\n", 
			vector.x, vector.y, vec2fLength(vector),
			normal.x, normal.y, vec2fLength(normal));

	Vec2f a = vec2f(3, 4);
	Vec2f sum = vec2fAdd(vector, a);
	printf("sum: %f, %f\n", sum.x, sum.y);


	Mat4f mat = mat4fIdentity();
	mat4fPrint(mat);	

	return 0;
}
