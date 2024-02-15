// Used for basic input/output stream
#include <stdio.h>
// Used for handling directory files
#include <dirent.h>
// For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <kernel.h>

void _cp(const char *target_file, const char *destination)
{
 int target_fd = sys_open(target_file, false);

 struct sys_stat *file_stat = malloc(sizeof(struct sys_stat));
 if (sys_stat(target_file, file_stat) == -1)
 {
  perror("Error in getting file status");
  exit(EXIT_FAILURE);
 }

 char *buf = malloc(file_stat->sys_size);
 if (!buf)
 {
  perror("Error in creating buffer");
  exit(EXIT_FAILURE);
 }

 sys_read(target_fd, buf, file_stat->sys_size);

 sys_close(target_fd);
 int dest_fd = sys_open(destination, true);

 sys_write(dest_fd, buf, file_stat->sys_size);

 free(buf);
 free(file_stat);

 sys_close(dest_fd);
}

int main(int argc, const char *argv[])
{
 if (argc < 3)
 {
  fprintf(stderr, "Usage: %s <filename> <destination>\n", argv[0]);
  return EXIT_FAILURE;
 }

 _cp(argv[1], argv[2]);

 return EXIT_SUCCESS;
}
