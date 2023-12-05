/* unit_disk.c: Unit tests for SimpleFS disk emulator */

#include "fs/disk.h"
#include "logging.h"
#include "test/minunit.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include <unistd.h>

/* Constants */
#define DISK_BLOCKS (4)

/* Functions */

int tests_run = 0;

static char* test_disk_open() {
    Disk* d = init_disk_struct(); 
    int res = disk_open(d, "thisdiskdoesnotexist");
    mu_assert("disk should not open\n", res == DISK_FAILURE);
    d->_isopen = 0;
    
    res = disk_open(d, "data/test/disk/good_disk_64b.vdisk");
    mu_assert("this disk should open correctly\n", res == DISK_SUCCESS);
    mu_assert("this disk should contains 64 blocks\n\n", d->_blocks == 64);
    res = disk_close(d);
    mu_assert("this disk should close correctly\n", res == DISK_SUCCESS);
    d = init_disk_struct();
    
    disk_open(d, "data/test/disk/good_disk_64b.vdisk");
    res = disk_open(d, "data/test/disk/good_disk_64b.vdisk");
    mu_assert("can't open a disk twice\n", res == DISK_FAILURE);
    disk_close(d); d = init_disk_struct();

    res = disk_open(d, "data/test/disk/bad_disk1.vdisk"); 
    mu_assert("this disk should not open correctly\n", res == DISK_FAILURE);
    disk_close(d);
    
    return EXIT_SUCCESS;
}

static char* test_disk_create() {
	char* path = "data/test/disk/disk_created.vdisk";
    Disk* d = init_disk_struct();
    int res = disk_create(path, 64);
    mu_assert("this disk should be created", res == DISK_SUCCESS);
    res = disk_open(d, path);
    mu_assert("this disk should open", res == DISK_SUCCESS);
    mu_assert("disk should have 64", disk_get_blocks(d) == 64);
    disk_close(d);
    
	unlink("data/test/disk/disk_created.vdisk"); // delete the file
    return EXIT_SUCCESS;
}

static char* test_disk_read() {
    char* path_test1 = "data/test/disk/disk_test_read1.vdisk";
    Disk* d = init_disk_struct();
    disk_open(d, path_test1);
    
    
    disk_close(d);
    return EXIT_SUCCESS;
}

static char* test_disk_write() {
    
    return EXIT_SUCCESS;
}

static char* all_tests() {
    mu_run_test(test_disk_open);
    mu_run_test(test_disk_create);
    mu_run_test(test_disk_read);
    mu_run_test(test_disk_write);
    return 0;
}

/* Main execution */

int main(int argc, char *argv[]) {
    //check if we are in the right path
    
    
    char *result = all_tests();
    if (result != 0) {
        printf("TEST ERROR : %s\n", result);
    }
    else {
        printf("----------- ALL TESTS PASSED ----------\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
