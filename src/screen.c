/* A prompt screen for the user to enter commands that are then executed
 * by the shell. */

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>

#ifdef RELEASE
int main(void)
{
	enable_logging_to_file();
	char *args = NULL; /* command line arguments */
	size_t args_length = 0;			   /* size of args buffer */
	ssize_t nread;					   /* number of characters read */
	int should_run = 1;				   /* flag to determine when to exit program */

	while (should_run)
	{
		fprintf(stdout, "osh>");
		
		if ((nread = getline(&args, &args_length, stdin)) == -1)
		{
			log_error("Error reading input...\n");
			exit(EXIT_FAILURE);
		}

		log_debug("args: %s", args);

		/* TODO: Temporary exit condition */
		if (strcmp(args, "exit\n") == 0)
		{
			should_run = 0;
		}

		/* TODO: Send the command to the shell */
	}
	
	log_debug("Freeing args...\n");
	free(args);
	close_log_file();

	return 0;
}
#endif
