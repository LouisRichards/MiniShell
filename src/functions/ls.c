// Used for basic input/output stream
#include <stdio.h>
// Used for handling directory files
#include <dirent.h>
// For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <kernel.h>

void _ls(const char *dir, int op_a, int op_l)
{
  struct sys_dirent *dir_entries = malloc(sizeof(struct sys_dirent));

  if (!sys_readdir(dir, dir_entries))
  {
    perror("Error in reading directory");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < dir_entries->entries; i++)
  {
    if (!op_a && dir_entries->names[i][0] == '.')
      continue;
    printf("%s  ", dir_entries->names[i]);
    if (op_l)
      printf("\n");
  }
  if (!op_l)
    printf("\n");

  free(dir_entries);
}
int main(int argc, const char *argv[])
{
  if (argc == 1)
  {
    _ls(".", 0, 0);
  }
  else if (argc == 2)
  {
    if (argv[1][0] == '-')
    {
      // Checking if option is passed
      // Options supporting: a, l
      int op_a = 0, op_l = 0;
      char *p = (char *)(argv[1] + 1);
      while (*p)
      {
        if (*p == 'a')
          op_a = 1;
        else if (*p == 'l')
          op_l = 1;
        else
        {
          perror("Option not available");
          exit(EXIT_FAILURE);
        }
        p++;
      }
      _ls(".", op_a, op_l);
    }
  }
  return 0;
}
