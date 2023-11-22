/* disk.c: SimpleFS disk emulator */

#include "sfs/disk.h"
#include "sfs/logging.h"

#include <fcntl.h>
#include <unistd.h>

/* Internal Prototyes */

bool disk_sanity_check(Disk *disk, size_t blocknum, const char *data);

/* External Functions */

/**
 *
 * Opens disk at specified path with the specified number of blocks by doing
 * the following:
 *
 *  1. Allocates Disk structure and sets appropriate attributes.
 *
 *  2. Opens file descriptor to specified path.
 *
 *  3. Truncates file to desired file size (blocks * BLOCK_SIZE).
 *
 * @param       path        Path to disk image to create.
 * @param       blocks      Number of blocks to allocate for disk image.
 *
 * @return      Pointer to newly allocated and configured Disk structure (NULL
 *              on failure).
 **/
Disk *disk_open(const char *path, size_t blocks)
{
    // Allocating Block structure
    Disk *disk = calloc(1, sizeof(Disk));
    if (!disk)
    {
        return NULL;
    }

    // Opening file descriptor and setting attributes
    int fd = open(path, O_RDWR | O_CREAT, 0600);
    if (fd < 0)
    {
        free(disk);
        return NULL;
    }

    disk->fd = fd;
    disk->blocks = blocks;
    disk->reads = 0;
    disk->writes = 0;
    disk->mounted = false;

    // Truncating file
    int truncated = ftruncate(disk->fd, (off_t)(blocks * BLOCK_SIZE));
    if (truncated != 0)
    {
        // Failed to truncate.
        close(disk->fd);
        free(disk);
        return NULL;
    }

    return disk;
}

/**
 * Close disk structure by doing the following:
 *
 *  1. Close disk file descriptor.
 *
 *  2. Report number of disk reads and writes.
 *
 *  3. Releasing disk structure memory.
 *
 * @param       disk        Pointer to Disk structure.
 */
void disk_close(Disk *disk)
{
    // Closing file descriptor
    close(disk->fd);

    // Reporting reads and writes
    printf("%lu reads\n", disk->reads);
    printf("%lu writes blocks\n", disk->writes);

    // Releasing disk structure memory
    free(disk);
}

/**
 * Read data from disk at specified block into data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified block.
 *
 *  3. Reading from block to data buffer (must be BLOCK_SIZE).
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes read.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_read(Disk *disk, size_t block, char *data)
{
    // Performing sanity check
    if (disk_sanity_check(disk, block, data) == false)
    {
        return DISK_FAILURE;
    }

    // Seeking to specified block
    off_t seek = lseek(disk->fd, (block * BLOCK_SIZE), SEEK_SET);
    if (seek < 0)
    {
        return DISK_FAILURE;
    }

    // Readin from block data to buffer
    ssize_t nread = read(disk->fd, data, BLOCK_SIZE);
    if (nread != BLOCK_SIZE)
    {
        return DISK_FAILURE;
    }

    disk->reads += 1;
    return BLOCK_SIZE;
}

/**
 * Write data to disk at specified block from data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified block.
 *
 *  3. Writing data buffer (must be BLOCK_SIZE) to disk block.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes written.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_write(Disk *disk, size_t block, char *data)
{
    // Performing sanity check
    if (disk_sanity_check(disk, block, data) == false)
    {
        return DISK_FAILURE;
    }

    // Seeking to specified block
    off_t seek = lseek(disk->fd, (block * BLOCK_SIZE), SEEK_SET);
    if (seek < 0)
    {
        return DISK_FAILURE;
    }

    // Writing from block data to buffer
    ssize_t nwrite = write(disk->fd, data, BLOCK_SIZE);
    if (nwrite != BLOCK_SIZE)
    {
        return DISK_FAILURE;
    }
    disk->writes += 1;
    return BLOCK_SIZE;
}

/* Internal Functions */

/**
 * Perform sanity check before read or write operation:
 *
 *  1. Check for valid disk.
 *
 *  2. Check for valid block.
 *
 *  3. Check for valid data.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Whether or not it is safe to perform a read/write operation
 *              (true for safe, false for unsafe).
 **/
bool disk_sanity_check(Disk *disk, size_t block, const char *data)
{
    // Checking for valid disk, block, and data
    return ((disk != NULL) && (block < disk->blocks && block >= 0) && (data != NULL));
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
