/* A prompt screen for the user to enter commands that are then executed
 * by the shell. */

#include <stdio.h>
#include <stdlib.h>
#include "../include/utils.h"

int main(void)
{
	char *args = malloc(sizeof(char)); /* command line arguments */
	int should_run = 1;				   /* flag to determine when to exit program */

	while (should_run)
	{
		fprintf(stderr, "osh>");
		fflush(stderr);

		size_t args_length = 0;
		if (getline(&args, &args_length, stdin) == -1)
		{
			log_error("Error reading input...");
			exit(EXIT_FAILURE);
		}

		log_debug("args: %s", args);

		/* TODO: Send the command to the shell */
	}

	free(args);

	return 0;
}
