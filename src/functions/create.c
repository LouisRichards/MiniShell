// Used for basic input/output stream
#include <stdio.h>
// Used for handling directory files
#include <dirent.h>
// For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <kernel.h>

/// @brief A create function that creates a file with the given size
/// and if the size is bigger than 0 fills it with numbers going from 0 to size
/// @param path
/// @param size
void _create(const char *path, size_t size)
{
 int fd = sys_open(path, true);
 if (fd == -1)
 {
  perror("Error in creating file");
  exit(EXIT_FAILURE);
 }
 if (size > 0)
 {
  char *buf = malloc(size);
  if (!buf)
  {
   perror("Error in creating buffer");
   exit(EXIT_FAILURE);
  }
  for (int i = 0; i < size; i++)
  {
   // Filling the buffer with numbers
   buf[i] = i % 10 + '0';
  }
  if (sys_write(fd, buf, size) == -1)
  {
   perror("Error in writing to file");
   exit(EXIT_FAILURE);
  }
  free(buf);
 }
 if (sys_close(fd) == -1)
 {
  perror("Error in closing file");
  exit(EXIT_FAILURE);
 }
}

int main(int argc, const char *argv[])
{
 if (argc < 2)
 {
  fprintf(stderr, "Usage: %s <filename> [size]\n", argv[0]);
  return EXIT_FAILURE;
 }
 size_t size = 0;
 if (argc == 3)
 {
  size = atoi(argv[2]);
 }
 _create(argv[1], size);
 return EXIT_SUCCESS;
}
