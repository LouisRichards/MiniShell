#include <stdio.h>
#include <kernel.h>
#include <errno.h>

void _cat(const char *pathname)
{
	struct sys_stat *file_stat = malloc(sizeof(struct sys_stat));
	if (!sys_stat(pathname, file_stat))
	{
		perror("Error in stat");
		exit(EXIT_FAILURE);
	}

	int size = file_stat->sys_size;
	
	int fd = sys_open(pathname, false);
 	if (fd == -1)
 	{
  		perror("Error in creating file");
  		exit(EXIT_FAILURE);
 	}
	
	char *buff = malloc(size);
	if (!buff)
	{
		perror("Error in creating buffer");
		exit(EXIT_FAILURE);
	}
	
	if (sys_read(fd, buff, size) == -1)
	{
		perror("Error in writing to file");
		exit(EXIT_FAILURE);
	}
	
	printf("%.*s\n",size,buff);
	
	sys_close(fd);
	
	free(buff);
}



int main(int argc, const char *argv[])
{
	if (argc < 1)
	{
		fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
		return EXIT_FAILURE;
 	}
 	
 	_cat(argv[1]);
 	return EXIT_SUCCESS;	
}
