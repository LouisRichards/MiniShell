/* disk.h: SimpleFS disk emulator */

#ifndef DISK_H
#define DISK_H

#include <stdbool.h>
#include <sys/types.h>

/* Disk Constants */

#define SECTOR_SIZE 4096

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
    size_t _sectors; /* Number of sectors in disk image	*/
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
 * Create an empty disk at specified path with the number of sectors specified.
 * it cannot replace a file if it's already present and will return an error code if trying to do so.
 * it will try to delete the file if it cannot write the disk it created
 *
 * @param       path        Path to disk image to create.
 * @param       sectors      Number of sectors to allocate for disk image.
 *
 * @return DISK_SUCCESS or DISK_FAILURE 
 **/
int disk_create(const char *path, size_t sectors);

/**
 *
 * Opens disk at specified path 
 *
 * @param       path        Path to disk image to create.
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
 * Read data from disk at specified sector into data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified sector.
 *
 *  3. Reading from sectors to data buffer (must be SECTOR_SIZE).
 *
 * @param       disk        Pointer to Disk structure.
 * @param       sector       Sector number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      DISK_FAILURE or DISK_IO_FAIL on failure, DISK_SUCCESS
 **/
int disk_read_sector(Disk *disk, unsigned char data[SECTOR_SIZE], size_t sector);

/**
 * Write data to disk at specified sector from data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified sector.
 *
 *  3. Writing data buffer (must be SECTOR_SIZE) to disk sector.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       sector    Sector number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes written.
 *              (SECTOR_SIZE on success, DISK_FAILURE on failure).
 **/
int disk_write_sector(Disk *disk, unsigned char data[SECTOR_SIZE], size_t sector);


/**
* Read up to count bytes from the disk at the desired offset
* Performs bound checking on the disk and throw an error if it happens
* @param    disk     Pointer to Disk structure
* @param    data    Data buffer to put the data on
* @param    count    number of bytes to read
* @param    offset    it starts to read at this offset
*
* @return DISK_SUCCESS or DISK_FAILURE
*    
**/
int disk_read_raw(Disk *disk, unsigned char *data, size_t count, size_t offset);

/**
* Write up to count bytes from the buffer at the disk offset
* Performs bound checking on the disk and throw an error if it happens
* @param    disk     Pointer to Disk structure
* @param    data    Data buffer to put the data on
* @param    count    number of bytes to read
* @param    offset    it starts to read at this offset
*
* @return DISK_SUCCESS or DISK_FAILURE
*    
**/
int disk_write_raw(Disk *disk, unsigned char *data, size_t count, size_t offset);



size_t disk_get_sectors(Disk *disk);
size_t disk_get_reads(Disk *disk);
size_t disk_get_writes(Disk *disk);
size_t disk_get_size(Disk *disk);
bool disk_is_open(Disk *disk);

#endif
