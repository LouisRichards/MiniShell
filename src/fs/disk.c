/* disk.c: simple disk emulator */

#include "fs/disk.h"
#include "utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* Internal Prototypes */

bool disk_sanity_check(Disk *disk, size_t sector, uint8_t *data);

/* External Functions */

Disk* init_disk_struct() {
    Disk* d = malloc(sizeof(Disk));
    d->_fd = -1;
    d->_reads = 0;
    d->_writes = 0;
    d->_size = 0;
    d->_isopen = 0;

    return d;
}

int disk_create(const char *path, size_t sectors) {
    char buf[SECTOR_SIZE] = {0};
    int fd = open(path, O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        log_error("cannot create disk: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    for (int i = 0; i < sectors; i++) {
        if (write(fd, buf, SECTOR_SIZE) != SECTOR_SIZE) {
            log_error("[disk_emu] cannot write disk: %s\n", strerror(errno));
            log_error("[disk_emu] trying to delete the disk");
            if (unlink(path) == -1) {
                log_error("[disk_emu] cannot delete the disk: %s\n", strerror(errno));
            }
            
            return DISK_FAILURE;
        }
    }

    return DISK_SUCCESS;
}

int disk_open(Disk *disk, const char *path)
{
    if (disk->_isopen) {
        log_error("[disk_emu] a disk is already opened, please close it first\n");
        return DISK_FAILURE;
    }

    // Opening file descriptor and setting attributes
    int fd = open(path, O_RDWR, 0600);
    if (fd < 0)
    {
        log_error("[disk_emu] cannot open the disk: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    // Check number of sector
    struct stat s;
    fstat(fd, &s);
    int size = s.st_size;
    log_info("[disk_emu] Disk size : %d\n", size);

    // it must contain an exact number of sectors
    if ((size % SECTOR_SIZE) != 0) {
        log_error("[disk_emu] the disk is corrupted\n");
        return DISK_FAILURE;
    }
    
    int sector_num = size / SECTOR_SIZE;
    log_info("[disk_emu] Disk sector number %d\n", sector_num);
    
    disk->_fd = fd;
    disk->_sectors = sector_num;
    disk->_reads = 0; //structure is supposed to be empty but this is just in case
    disk->_writes = 0;
    disk->_size = size;
    disk->_isopen = 1;
    
    return DISK_SUCCESS;
}

int disk_close(Disk *disk)
{    
    // Closing file descriptor
    if (disk->_fd != -1) {
       if (close(disk->_fd) == -1) {
            log_error("[disk_emu] cannot close the disk: %s\n", strerror(errno));
            return DISK_FAILURE;
        } 
    }
    

    // Reporting reads and writes
    log_info("[disk_emu] %lu reads\n", disk->_reads);
    log_info("[disk_emu] %lu writes\n", disk->_writes);

    // Releasing disk structure memory
    free(disk);

    return DISK_SUCCESS;
}


int disk_read_sector(Disk *disk, uint8_t data[SECTOR_SIZE], size_t sector)
{
    // Performing sanity check
    if (disk_sanity_check(disk, sector, data) == false) {
        log_error("[disk_emu] Error, read operation failed on disk\n");
        return DISK_FAILURE;
    }

    // Seeking to specified sector
    off_t seek = lseek(disk->_fd, (sector * SECTOR_SIZE), SEEK_SET);
    if (seek < 0) {
        log_error("[disk_emu] cannot seek into the disk");
        return DISK_IO_FAIL;
    }

    // Reading from sector data to buffer
    ssize_t nread = read(disk->_fd, data, SECTOR_SIZE);
    if (nread != SECTOR_SIZE) {
        return DISK_IO_FAIL;
    }

    disk->_reads += 1;
    return DISK_SUCCESS;
}

int disk_write_sector(Disk *disk, uint8_t data[SECTOR_SIZE], size_t sector)
{
    // Performing sanity check
    if (disk_sanity_check(disk, sector, data) == false) {
        return DISK_FAILURE;
    }

    // Seeking to specified sector
    off_t seek = lseek(disk->_fd, (sector * SECTOR_SIZE), SEEK_SET);
    if (seek < 0) {
        log_error("[disk_emu] cannot seek into the disk\n");
        return DISK_IO_FAIL;
    }

    // Writing from sector data to buffer
    ssize_t nwrite = write(disk->_fd, data, SECTOR_SIZE);
    if (nwrite != SECTOR_SIZE) {
        log_error("[disk_emu] cannot write the disk");
        return DISK_IO_FAIL;
    }
    disk->_writes += 1;
    return DISK_SUCCESS;
}

int disk_read_raw(Disk *disk, uint8_t *data, size_t count, size_t offset) {
    if (offset + count > disk->_size) {
        log_error("[disk_emu] trying to read outside of the disk\n");
        return DISK_FAILURE;
    }

    if (lseek(disk->_fd, offset, SEEK_SET) < 0) {
        log_error("[disk_emu] cannot seek into the disk\n");
        return DISK_IO_FAIL;
    }

    if (read(disk->_fd, data, count) != count) {
        log_error("[disk_emu] cannot read the disk");
        return DISK_IO_FAIL;
    }

    disk->_reads += 1;
    return DISK_SUCCESS;
}

int disk_write_raw(Disk *disk, uint8_t *data, size_t count, size_t offset) {
    if (offset + count > disk->_size) {
        log_error("[disk_emu] trying to write outside of the disk");
        return DISK_FAILURE;
    }

    if (lseek(disk->_fd, offset, SEEK_SET) < 0) {
        log_error("[disk_emu] cannot seek into the disk");
        return DISK_IO_FAIL;
    }

    if (write(disk->_fd, data, count) != count) {
        log_error("[disk_emu] cannot write the disk");
        return DISK_IO_FAIL;
    }

    disk->_writes += 1;
    return DISK_SUCCESS;
}

size_t disk_get_reads(Disk *disk) 
{
    return disk->_reads;
}

size_t disk_get_writes(Disk *disk) 
{
    return disk->_reads;
}

size_t disk_get_size(Disk *disk) 
{
    return disk->_size;
}

bool disk_is_open(Disk *disk) 
{
    return disk->_isopen;
}

size_t disk_get_sectors(Disk *disk) 
{
    return disk->_sectors;
}

/* Internal Functions */
bool disk_sanity_check(Disk *disk, size_t sector, uint8_t *data) 
{
    // Checking for valid disk, sector, and data
    return ((disk != NULL) && (sector < disk->_sectors && sector >= 0) && (data != NULL));
}

