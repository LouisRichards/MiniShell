/* disk.c: simple disk emulator */

#include "fs/disk.h"
#include "logging.h"
#include "utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

/* Internal Prototyes */

bool disk_sanity_check(Disk *disk, size_t blocknum, const char *data);

/* External Functions */

Disk* init_disk_struct() {
    Disk* d = malloc(sizeof(Disk));
    d->_fd = 0;
    d->_reads = 0;
    d->_writes = 0;
    d->_size = 0;
    d->_isopen = 0;

    return d;
}

int disk_create(const char *path, size_t blocks) {
    char buf[BLOCK_SIZE] = {0};
    int fd = open(path, O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        log_error("cannot create disk: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    for (int i = 0; i < blocks; i++) {
        if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE) {
            log_error("cannot create disk: %s\n", strerror(errno));
        }
    }

    return DISK_SUCCESS;
}

int disk_open(Disk *disk, const char *path)
{
    
    if (disk->_isopen) {
        log_error("a disk is already opened, please close it first\n");
        return DISK_FAILURE;
    }

    // Opening file descriptor and setting attributes
    int fd = open(path, O_RDWR, 0600);
    if (fd < 0)
    {
        log_error("cannot open the disk: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    // Check number of block
    struct stat s;
    fstat(fd, &s);
    int size = s.st_size;
    log_info("Disk size : %d\n", size);

    // it must contain an exact number of blocks
    if ((size % BLOCK_SIZE) != 0) {
        log_error("the disk is corrupted\n");
        return DISK_FAILURE;
    }
    
    int block_num = size / BLOCK_SIZE;
    log_info("Disk block number %d\n", block_num);
    
    disk->_fd = fd;
    disk->_blocks = block_num;
    disk->_reads = 0; //structure is supposed to be empty but this is just in case
    disk->_writes = 0;
    disk->_size = size;
    disk->_isopen = 1;
    
    return DISK_SUCCESS;
}

int disk_close(Disk *disk)
{
    // Closing file descriptor
    if (close(disk->_fd) == -1) {
        log_error("cannot close the disk: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    // Reporting reads and writes
    log_info("%lu reads\n", disk->_reads);
    log_info("%lu writes blocks\n", disk->_writes);

    // Releasing disk structure memory
    free(disk);

    return DISK_SUCCESS;
}


int disk_read(Disk *disk, size_t block, char *data)
{
    // Performing sanity check
    if (disk_sanity_check(disk, block, data) == false)
    {
        log_error("Error, read operation failed on disk\n");
        return DISK_FAILURE;
    }

    // Seeking to specified block
    off_t seek = lseek(disk->_fd, (block * BLOCK_SIZE), SEEK_SET);
    if (seek < 0)
    {
        return DISK_IO_FAIL;
    }

    // Readin from block data to buffer
    ssize_t nread = read(disk->_fd, data, BLOCK_SIZE);
    if (nread != BLOCK_SIZE)
    {
        return DISK_IO_FAIL;
    }

    disk->_reads += 1;
    return DISK_SUCCESS;
}


int disk_write(Disk *disk, size_t block, char *data)
{
    // Performing sanity check
    if (disk_sanity_check(disk, block, data) == false)
    {
        return DISK_FAILURE;
    }

    // check if we are writing outside the disk
    if ((block*disk->_blocks + strlen(data)) > disk->_size) {
        log_error("trying to write outside of the disk\n");
        return DISK_IO_FAIL;
    }

    // Seeking to specified block
    off_t seek = lseek(disk->_fd, (block * BLOCK_SIZE), SEEK_SET);
    if (seek < 0)
    {
        return DISK_IO_FAIL;
    }

    // Writing from block data to buffer
    ssize_t nwrite = write(disk->_fd, data, BLOCK_SIZE);
    if (nwrite != BLOCK_SIZE)
    {
        return DISK_IO_FAIL;
    }
    disk->_writes += 1;
    return BLOCK_SIZE;
}

/* Internal Functions */
bool disk_sanity_check(Disk *disk, size_t block, const char *data)
{
    // Checking for valid disk, block, and data
    return ((disk != NULL) && (block < disk->_blocks && block >= 0) && (data != NULL));
}

size_t disk_get_reads(Disk *disk) {
    return disk->_reads;
}

size_t disk_get_writes(Disk *disk) {
    return disk->_reads;
}

size_t disk_get_size(Disk *disk) {
    return disk->_size;
}

bool disk_is_open(Disk *disk) {
    return disk->_isopen;
}

size_t disk_get_blocks(Disk *disk) {
    return disk->_blocks;
}
