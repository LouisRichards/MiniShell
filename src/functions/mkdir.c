#include <stdio.h>
#include <kernel.h>
#include <errno.h>

void _mkdir(const char *pathname)
{

	sys_mkdir(pathname,0760);

}

int main(int argc, const char *argv[])
{
	if (argc < 1)
	{
		fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
		return EXIT_FAILURE;
 	}
 	
 	_mkdir(argv[1]);
 	return EXIT_SUCCESS;	
}
