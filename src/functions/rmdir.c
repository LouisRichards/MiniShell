#include <stdio.h>
#include <kernel.h>
#include <errno.h>

void _rmdir(const char *pathname)
{

	sys_rmdir(pathname);

}

int main(int argc, const char *argv[])
{
	if (argc < 1)
	{
		fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
		return EXIT_FAILURE;
 	}
 	
 	_rmdir(argv[1]);
 	return EXIT_SUCCESS;	
}
