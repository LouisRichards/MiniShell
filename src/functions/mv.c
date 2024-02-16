// Used for basic input/output stream
#include <stdio.h>
// Used for handling directory files
#include <dirent.h>
// For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <kernel.h>

void _mv(const char *target_file, const char *destination)
{
 if (sys_rename(target_file, destination) == -1)
 {
  perror("Error in moving file");
  exit(EXIT_FAILURE);
 }
}

int main(int argc, const char *argv[])
{
 if (argc < 3)
 {
  fprintf(stderr, "Usage: %s <filename> <destination>\n", argv[0]);
  return EXIT_FAILURE;
 }

 _mv(argv[1], argv[2]);

 return EXIT_SUCCESS;
}
