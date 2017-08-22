#include <stdio.h>
#include "texture.h"
#include "file_get.h"
#include "upng/upng.h"

Texture *loadTexture(const char *filePath) 
{
	Texture *texture = NULL;
	unsigned char *buff = NULL;
	int size;
	buff = file_get(filePath, &size);

	upng_t *png = upng_new_from_bytes(buff, size);
	if(!png) {
		fprintf(stderr, "Cannot convert png\n");
		free(buff);
		upng_free(png);
		return NULL;
	}

	upng_decode(png);
	if(upng_get_error(png) == UPNG_EOK) {
		
		texture = calloc(1, sizeof(Texture));
		if(!texture) {
			fprintf(stderr, "Cannot alloc texture\n");
			return NULL;
		}
		texture->width = upng_get_width(png);
		texture->height = upng_get_height(png);

		glGenTextures(1, &texture->id);
		glBindTexture(GL_TEXTURE_2D, texture->id);
		 // upload texture //
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, upng_get_buffer(png));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
	}
	upng_free(png);
	free(buff);

	return texture;	
}

