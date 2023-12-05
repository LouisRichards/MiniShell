/* disk.h: SimpleFS disk emulator */

#ifndef DISK_H
#define DISK_H

#include <stdbool.h>
#include <sys/types.h>

/* Disk Constants */

#define BLOCK_SIZE 4096

/* Return Codes */

enum {
    /* those are generic codes */
    DISK_SUCCESS,
    DISK_FAILURE,

    /* specific codes */
    DISK_IO_FAIL
};

/* Disk Structure */

typedef struct Disk Disk;

struct Disk
{
    int _fd;        /* File descriptor of disk image	*/
    size_t _blocks; /* Number of blocks in disk image	*/
    size_t _reads;  /* Number of reads to disk image	*/
    size_t _writes; /* Number of writes to disk image	*/
    size_t _size; /* size of disk in bytes */
    bool _isopen; /* is a disk opened ? */
};

/* Disk Functions */

/**
* Allocate and returns an empty Disk structure.
* You should not try to allocate this structure yourself when passing it in functions,
* so use this function when you need it.
*/
Disk* init_disk_struct();

/**
 *
 * Create an empty disk at specified path with the number of blocks specified.
 * it cannot replace a file if it's already present and will return an error code if trying to do so.
 *
 * @param       path        Path to disk image to create.
 * @param       blocks      Number of blocks to allocate for disk image.
 *
 * @return DISK_SUCCESS or DISK_FAILURE 
 **/
int disk_create(const char *path, size_t blocks);

/**
 *
 * Opens disk at specified path 
 *
 * @param       path        Path to disk image to create.
 * @param       blocks      Number of blocks to allocate for disk image.
 * @param       disk        previously allocated pointer of the disk to open 
 * @return DISK_SUCCESS or DISK_FAILURE 
 **/
int disk_open(Disk *disk, const char *path);

/**
 * Close disk and free Disk struct. it will not free the Disk stucture if it cannot close the file descriptor
 * user responsability is to CHECK if it was successfull or not.
 *
 * @param       disk        Pointer to Disk structure.
 * @return DISK_SUCCESS or DISK_FAILURE if it cannot close it 
 */
int disk_close(Disk *disk);

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
 * @return      DISK_FAILURE or DISK_IO_FAIL on failure, DISK_SUCCESS
 **/
int disk_read(Disk *disk, size_t block, char *data);

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
int disk_write(Disk *disk, size_t block, char *data);

size_t disk_get_blocks(Disk *disk);
size_t disk_get_reads(Disk *disk);
size_t disk_get_writes(Disk *disk);
size_t disk_get_size(Disk *disk);
bool disk_is_open(Disk *disk);

#endif
