#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"

#define RESPONSE_FIFO_NAME "fromShellFIFO"
#define BUFFER_SIZE 256

// Function to handle user input and provide response
void processUserInput(int should_pipe);

int main(int argc, char *argv[]) {
    int should_pipe = 0;

    // Check if the user wants to pipe the output
    if (argc > 1 && strcmp(argv[1], "-p") == 0) {
        should_pipe = 1;
    }

    // Create FIFOs (named pipes) if piping is requested
    if (should_pipe) {
        if (mknod(RESPONSE_FIFO_NAME, S_IFIFO | 0666, 0) == -1) {
        	log_warning("Pipe already exists, using it!\n");
		}
    }

    // Process user input and provide response
    processUserInput(should_pipe);

    // Cleanup: Close FIFO if used
    if (should_pipe) {
        if (unlink(RESPONSE_FIFO_NAME) == -1) {
            perror("Error removing named pipe");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

void processUserInput(int should_pipe) {
    char userInput[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int fromShellPipe;

    // Open the named pipe for writing if piping is requested
    if (should_pipe) {
        fromShellPipe = open(RESPONSE_FIFO_NAME, O_WRONLY);
        if (fromShellPipe == -1) {
            perror("Error opening named pipe");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        printf("LLP $ ");
        fgets(userInput, BUFFER_SIZE, stdin);

        // Remove the newline character at the end of user input
        userInput[strlen(userInput) - 1] = '\0';

        // Log user input
        log_info("User input: %s\n", userInput);

        // Get command response (temporary response is set to userInput)
        strcpy(response, userInput);

        // Write response back to the user prompt if piping is requested
        if (should_pipe) {
            if (write(fromShellPipe, response, strlen(response) + 1) == -1) {
                perror("Error writing to named pipe");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Close the named pipe if used
    if (should_pipe) {
        close(fromShellPipe);
    }
}

