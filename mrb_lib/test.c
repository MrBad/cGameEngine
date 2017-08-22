#include "file_get.h"

int main()
{
	unsigned char *buff = NULL;
	int size;

	buff = file_get("../shaders/color_shader.fs", &size);
	printf ("%p\n", buff);

	printf("size: %d\n", size);

	printf("---\n%s\n", buff);


	free(buff);
}
