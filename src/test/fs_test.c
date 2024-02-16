#include "fs/fs.h"
#include "fs/disk.h"

#include "utils.h"
#include "test/minunit.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include <unistd.h>

int tests_run = 0;


static char* test_inode_save_load() {

	return EXIT_SUCCESS;
}

static char* all_tests() {

	mu_run_test(test_inode_save_load);

	return EXIT_SUCCESS;
}

int main() {

	char* result = all_tests();

	if (result != EXIT_SUCCESS) {
		printf("TEST ERROR : %s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}
	
	printf("Tests run: %d\n", tests_run);

	return result != EXIT_SUCCESS;
}
