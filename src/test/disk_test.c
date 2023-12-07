/* unit_disk.c: Unit tests for SimpleFS disk emulator */

#include "fs/disk.h"
#include "test/minunit.h"
#include "utils.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int tests_run = 0;

static char* test_disk_open() {
    Disk* d = init_disk_struct(); 
    int res = disk_open(d, "thisdiskdoesnotexist");
    mu_assert("disk should not open\n", res == DISK_FAILURE);
    d->_isopen = 0;
    
    res = disk_open(d, "data/test/disk/good_disk_64b.vdisk");
    mu_assert("this disk should open correctly\n", res == DISK_SUCCESS);
    mu_assert("this disk should contains 64 sectors\n\n", d->_sectors == 64);
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
    mu_assert("disk should have 64", disk_get_sectors(d) == 64);
    disk_close(d);
    
	unlink(path); // delete the file
    return EXIT_SUCCESS;
}

static char* test_disk_read() {
	unsigned char buf[SECTOR_SIZE] = {0};
    char* path_test1 = "data/test/disk/disk_test_read1.vdisk";
    Disk* d = init_disk_struct();
    
    disk_open(d, path_test1);
	disk_read_sector(d, buf, 0);
	mu_assert("sector should begin with 0xDEADBEEF", buf[0] == 0xDE && buf[1] == 0xAD && buf[2] == 0xBE && buf[3] == 0xEF);
	
	disk_read_sector(d, buf, 5);
	unsigned char* test_string_buf = buf + 388; 
    mu_assert("sector should contain \"test string\"", strncmp("test string", (char*) test_string_buf, 11) == 0);
    
    //trying to read outside the limits
    mu_assert("shouldn't read non existing sector ", disk_read_sector(d, buf, 33) != DISK_SUCCESS);
    mu_assert("shouldn't read non existing sector ", disk_read_sector(d, buf, -1) != DISK_SUCCESS);

    mu_assert("should read this", disk_read_raw(d, buf, 1, 135167) == DISK_SUCCESS);
    mu_assert("should read this", disk_read_raw(d, buf, 1, 0) == DISK_SUCCESS);
    mu_assert("shouldn't read this", disk_read_raw(d, buf, 2, 135167) != DISK_SUCCESS);
    
    
    disk_close(d);
    return EXIT_SUCCESS;
}

static char* test_disk_write() {
    unsigned char buf[SECTOR_SIZE] = {0};
    char* path_test = "data/test/disk/disk_created_write.vdisk";
    mu_assert("cannot create test disk", disk_create(path_test, 45) == DISK_SUCCESS);
    Disk *disk = init_disk_struct();
    disk_open(disk, path_test);

    for (int b = 0; b < 45; b++) {
        memset(buf, b, SECTOR_SIZE);
        mu_assert("should write block", disk_write_sector(disk, buf, b) == DISK_SUCCESS);
        mu_assert("should read block", disk_read_sector(disk, buf, b) == DISK_SUCCESS);
        for (int i = 0; i < SECTOR_SIZE; i++) {
            mu_assert("data should be equals", buf[i] == b);
        }
        
        mu_assert("should write raw", disk_write_raw(disk, buf, SECTOR_SIZE, b*SECTOR_SIZE) == DISK_SUCCESS);
        mu_assert("should read block", disk_read_sector(disk, buf, b) == DISK_SUCCESS);
        for (int i = 0; i < SECTOR_SIZE; i++) {
            mu_assert("data should be equals", buf[i] == b);
        }
        memset(buf, 0, SECTOR_SIZE);
        
    }
    mu_assert("disk writes should be logged", disk_get_writes(disk) == 90);
    mu_assert("shouldn't write outside", disk_write_sector(disk, buf, 46) == DISK_FAILURE);
    
    
    disk_close(disk);
    unlink(path_test);
    return EXIT_SUCCESS;
}

static char* all_tests() {
    printf("TESTING DISK OPEN\n");
    mu_run_test(test_disk_open);
    printf("TESTING DISK CREATE\n");
    mu_run_test(test_disk_create);
    printf("TESTING DISK READ\n");
    mu_run_test(test_disk_read);
    printf("TESTING DISK WRITE\n");
    mu_run_test(test_disk_write);
    return 0;
}


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
