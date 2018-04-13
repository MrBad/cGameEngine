#ifndef FILE_GET_H
#define FILE_GET_H

/**
 * Gets the content of a file
 *
 * @param path The file path
 * @param size A pointer to an int to save the buffer size
 * @return a new buffer with the contents of the file or NULL on error.
 *      This buffer must be freed.
 */
unsigned char *file_get(const char *path, int *size);

#endif
