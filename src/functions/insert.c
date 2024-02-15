// Used for basic input/output stream
#include <stdio.h>
// Used for handling directory files
#include <dirent.h>
// For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <kernel.h>
#include <string.h>

/// @brief A insert function that adds content to a file
/// @param path
/// @param size
void _insert(const char *file_path, const char *content)
{

 // Calculate content size based on the first and last quote
 size_t size = strlen(content) + 1; // +1 for the null terminator

 int fd = sys_open(file_path, false);
 if (fd == -1)
 {
  perror("Error in opening file");
  exit(EXIT_FAILURE);
 }

 sys_write(fd, content, size);

 sys_close(fd);
}

int main(int argc, const char *argv[])
{
 if (argc < 3)
 {
  fprintf(stderr, "Usage: %s <filename> <content>\n", argv[0]);
  return EXIT_FAILURE;
 }

 _insert(argv[1], argv[2]);

 return EXIT_SUCCESS;
}
