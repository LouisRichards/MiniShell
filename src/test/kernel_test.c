#include "kernel.h"

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

static char* test_kernel_open() {

	int fd = sys_open("./data/test/kernel_test", O_RDONLY);

	mu_assert("Error opening file", fd>0);

	return EXIT_SUCCESS;
}

static char* test_kernel_close() {

	int fd = sys_open("./data/test/kernel_test", O_RDONLY);

	mu_assert("Error opening file", fd>0);

	int ret = sys_close(fd);

	mu_assert("Error closing file", ret==0);

	return EXIT_SUCCESS;
}

static char* test_kernel_read() {

	int fd = sys_open("./data/test/kernel_test", O_RDONLY);

	mu_assert("Error opening file", fd>0);

	char buf[36];

	int ret = sys_read(fd, buf, 35);

	buf[ret] = '\0';

	mu_assert("Error reading file", strcmp(buf, "Ceci est un message du gouvernement")==0);

	return EXIT_SUCCESS;
}

static char* test_kernel_write() {

	int fd = sys_open("./data/test/kernel_test", O_WRONLY);

	mu_assert("Error opening file", fd>0);

	char buf[10] = "Ceci est ";

	int ret = sys_write(fd, buf, 9);

	mu_assert("Error writing file", strcmp(buf, "Ceci est ")==0);

	return EXIT_SUCCESS;
}

static char* test_kernel_lseek() {

	int fd = sys_open("./data/test/kernel_test", O_RDONLY);

	mu_assert("Error opening file", fd>0);

	char buf[36];

	int ret = sys_read(fd, buf, 35);

	buf[ret] = '\0';

	mu_assert("Error reading file", strcmp(buf, "Ceci est un message du gouvernement")==0);

	ret = sys_lseek(fd, 0, SEEK_SET);

	mu_assert("Error seeking file", ret==0);

	ret = sys_read(fd, buf, 35);

	buf[ret] = '\0';

	mu_assert("Error reading file", strcmp(buf, "Ceci est un message du gouvernement")==0);

	return EXIT_SUCCESS;
}

static char* all_tests() {

	mu_run_test(test_kernel_open);
	mu_run_test(test_kernel_close);
	mu_run_test(test_kernel_read);
	mu_run_test(test_kernel_write);
	mu_run_test(test_kernel_lseek);

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
