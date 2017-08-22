#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// returns size or -1 on error
// content will be placed in alloced buffer
unsigned char *file_get(const char *path, int *size)
{
	struct stat st;
	FILE *fp;
	int n_bytes, n_read, left;

	unsigned char *buff = NULL;

	if(stat(path, &st) < 0) {
		fprintf(stderr, "Cannot stat %s\n", path);
		return NULL;
	}
	*size = st.st_size;
	buff = malloc(*size + 1);
	printf("%p\n", buff);
	if(!buff) {
		fprintf(stderr, "Cannot malloc buffer\n");
		return NULL;
	}

	if(!(fp = fopen(path, "r"))) {
		fprintf(stderr, "Cannot open: %s\n", path);
		return NULL;
	}
	n_read = 0;
	left = *size;
	while(left > 0) {
		n_bytes = fread(buff + n_read, 1, left, fp);
		left -= n_bytes;
		n_read += n_bytes;
	}
	fclose(fp);
	buff[*size] = 0;
	return buff;

}
