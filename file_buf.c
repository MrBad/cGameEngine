#include "file_buf.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// maybe i should rewrite this into a more simple file_get_contents() like

FileBuf *fileBufNew(const char *path)
{
	struct stat st;

	if(stat(path, &st) < 0) {
		fprintf(stderr, "Cannot stat: %s\n", path);
		return NULL;
	}

	FileBuf *fBuf = calloc(1, sizeof(fBuf));
	if(!fBuf) {
		fprintf(stderr, "Cannot calloc fBuf\n");
		return NULL;
	}

	// c11 does not know about strdup
	fBuf->path = calloc(1, strlen(path) + 1);
	if(!fBuf->path) {
		fprintf(stderr, "Cannot calloc fBuf->path\n");
		free(fBuf);
		return NULL;
	}
	strcpy(fBuf->path, path);
	fBuf->size = st.st_size;
	fBuf->data = malloc(fBuf->size + 1);
	if(!fBuf->data)  {
		fprintf(stderr, "Cannot malloc fBuf->data\n");
		free(fBuf->path);
		free(fBuf);
		return NULL;
	}
	fBuf->data[fBuf->size] = 0; // just in case
	return fBuf;
}

int fileBufLoad(FileBuf *fBuf)
{
	FILE *fp;
	int n_bytes, n_read, left;
	if(!(fp = fopen(fBuf->path, "r"))) {
		fprintf(stderr, "Cannot open: %s\n", fBuf->path);
		return -1;
	}
	n_read = 0;
	left = fBuf->size;
	while(left > 0) {
		n_bytes = fread(fBuf->data + n_read, 1, left, fp);
		left -= n_bytes;
		n_read += n_bytes;
	}	
	fclose(fp);
	return 0;
}

void fileBufDelete(FileBuf *fBuf)
{
	if(fBuf->path) 
		free(fBuf->path);
	if(fBuf->data) 
		free(fBuf->data);
	free(fBuf);
	fBuf = NULL;
}
