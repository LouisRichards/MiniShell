/* fs.h: SimpleFS file system */

#ifndef FS_H
#define FS_H

#include "./disk.h"

#include <stdbool.h>
#include <stdint.h>

/*     File System Constants 
    /!\ run test when changing /!\ */ 
    
enum FsConstants {
	FS_BLOCK_SIZE =4096,
	FS_MAGIC_NUMBER = 0x12ABCDEF,
	FS_INODE_BLOCKS = 512,
	FS_INODE_BITMAP_LENGTH = 1,
	FS_DATA_BITMAP_LENGTH = 1,
	FS_MAX_PATH_LENGTH = 512,
	
	//inode settings
	FS_POINTERS_PER_INODE = 8,  // Number of direct pointers per inode 
	FS_INDIRECT_POINTERS_PER_INODE = 4,  // Number of indirect pointers per inode 
	FS_INODE_SIZE = 128, // size of an inode structure in bytes (DO NOT change or it will break)
	FS_POINTER_SIZE = 4, // size of a pointer in byte (DO NOT change or it will break)
	
	//folder settings
	FS_MAX_FILENAME_LENGTH = 16, // DO NOT CHANGE THIS PLEASE
	FS_DIR_SIZE = 32,

	/* Computed constants */
	FS_POINTER_PER_BLOCK = FS_BLOCK_SIZE/FS_POINTER_SIZE,
	FS_INODE_BITMAP_START_ADDR = FS_BLOCK_SIZE,
	FS_DATA_BITMAP_START_ADDR = FS_BLOCK_SIZE + (FS_INODE_BITMAP_LENGTH * FS_BLOCK_SIZE),
	FS_INODE_TABLE_START_ADDR = FS_BLOCK_SIZE + (FS_INODE_BITMAP_LENGTH * FS_BLOCK_SIZE) + (FS_DATA_BITMAP_LENGTH * FS_BLOCK_SIZE), //offset of inode table in the partition
	FS_DATA_START_ADDR = FS_INODE_TABLE_START_ADDR+(FS_BLOCK_SIZE * FS_INODE_BLOCKS),

	FS_MAX_INODES = (FS_BLOCK_SIZE / FS_INODE_SIZE) * FS_INODE_BLOCKS, //maximum number of inodes in the system
	FS_MAX_FILE_SIZE = (FS_POINTERS_PER_INODE * FS_BLOCK_SIZE) + (FS_INDIRECT_POINTERS_PER_INODE * ((FS_BLOCK_SIZE/FS_POINTER_SIZE) * FS_BLOCK_SIZE))
};



/* File System Structures */
typedef int inode_t;

typedef struct InodeInfo InodeInfo;
struct InodeInfo {
    uint8_t type; // format of the file, see fs_filetype enumeration
    uint64_t size; // the size of the file in bytes
    uint64_t atime; // last time the file was accessed (UNIX time)
    uint64_t mtime; // last time the file was modified (UNIX time)
    uint64_t blocks; // how many data blocks the inode is taking
};

typedef struct FileSystem FileSystem;
struct FileSystem
{
    Disk *disk;           /* Disk file system is mounted on */
};

enum fs_filetype {
    FS_TYPE_NONE = 0xFF, // deleted or non existant file, you can overwrite the inode
    FS_TYPE_FILE = 0xF1, // regular file
    FS_TYPE_LINK = 0x12, // symbolic link
    FS_TYPE_DIR  = 0xD1 // directory
};


/* File System Functions */

/**
 * Debug FileSystem by doing the following
 *
 *  1. Read SuperBlock and report its information.
 *
 *  2. Read Inode Table and report information about each Inode.
 *
 * @param       disk        Pointer to Disk structure.
 **/
void fs_debug(Disk *disk);

/**
 * Format Disk by doing the following:
 *
 *  1. Write SuperBlock (with appropriate magic number, number of blocks,
 *  number of inode blocks, and number of inodes).
 *
 *  2. Clear all remaining blocks.
 *
 * Note: Do not format a mounted Disk!
 *
 * @param       disk        Pointer to Disk structure.
 * @param       quick_format Do not overwrite all the data blocks, if false it will put all the data blocks to 0
 * @return      Whether or not all disk operations were successful.
 **/
bool fs_format(Disk *disk, bool quick_format);

/**
 * Mount specified FileSystem to given Disk by doing the following:
 *
 *  1. Read and check SuperBlock (verify attributes).
 *
 *  2. Record FileSystem disk attribute and set Disk mount status.
 *
 *  3. Copy SuperBlock to FileSystem meta data attribute
 *
 *  4. Initialize FileSystem free blocks bitmap.
 *
 * Note: You cannot mount a Disk that has already been mounted!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not the mount operation was successful.
 **/
bool fs_mount(FileSystem *fs, Disk *disk);

/**
 * Unmount FileSystem from internal Disk by doing the following:
 *
 *  1. Set Disk mounted status and FileSystem disk attribute.
 *
 *  2. Release free blocks bitmap.
 *
 * @param       fs      Pointer to FileSystem structure.
 **/
void fs_unmount(FileSystem *fs);

inode_t fs_create_file(FileSystem *fs, char *path);

inode_t fs_mkdir(FileSystem *fs, char *path);

bool fs_rmdir(FileSystem *fs, inode_t dir);

bool fs_unlink(FileSystem *fs, inode_t file);


/**
 * Read from the specified Inode into the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously read blocks and copy data to buffer.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to read data from.
 * @param       data            Buffer to copy data to.
 * @param       length          Number of bytes to read.
 * @param       offset          Byte offset from which to begin reading.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_read(FileSystem *fs, size_t inode_number, uint8_t *data, size_t length, size_t offset);

/**
 * Write to the specified Inode from the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously copy data from buffer to blocks.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to write data to.
 * @param       data            Buffer with data to copy
 * @param       length          Number of bytes to write.
 * @param       offset          Byte offset from which to begin writing.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_write(FileSystem *fs, size_t inode_number, uint8_t *data, size_t length, size_t offset);

ssize_t fs_rename(char* oldpath, char* newpath);

bool fs_truncate(FileSystem *fs, inode_t file, size_t newsize);

char*fs_readdir(FileSystem *fs, inode_t dir); // retourne des chemins absolus

inode_t fs_getinode(FileSystem *fs, char* path);

InodeInfo fs_getinfo(FileSystem *fs, inode_t inode);



#endif
