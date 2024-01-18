#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FIFO_NAME "toShellFIFO"
#define RESPONSE_FIFO_NAME "fromShellFIFO"
#define BUFFER_SIZE 256

int main() {
	char shellResponse[BUFFER_SIZE];

    // Create FIFOs (named pipes)
    mknod(RESPONSE_FIFO_NAME, S_IFIFO | 0666, 0);

    int fromShellPipe = open(RESPONSE_FIFO_NAME, O_RDONLY);

    while (1) {
        // Read response from the shell
        read(fromShellPipe, shellResponse, BUFFER_SIZE);

        // Display shell response
        printf("%s\n", shellResponse);
    }

	close(fromShellPipe);

    return 0;
}

