/* fs.h: SimpleFS file system */

#ifndef FS_H
#define FS_H

#include "./disk.h"

#include <stdbool.h>
#include <stdint.h>



/* File System Constants */ /* /!\ please DO NOT change /!\ */
#define FS_BLOCK_SIZE 4096
#define FS_MAGIC_NUMBER 0x12ABCDEF
#define FS_INODE_BLOCKS 512 // Number of blocks reserved for the inode table
#define FS_INODE_BITMAP_LENGTH 1 // size of the inode table bitmap (in blocks)
#define FS_DATA_BITMAP_LENGTH 1 // size of the data bitmap (in blocks)
//inode settings
#define FS_POINTERS_PER_INODE 8  // Number of direct pointers per inode 
#define FS_INDIRECT_POINTERS_PER_INODE 4  // Number of indirect pointers per inode 
#define FS_INODE_SIZE 128 // size of an inode structure in bytes

/* Computed constants */
//offset of inode table in the partition
#define FS_INODE_TABLE_START_ADDR FS_BLOCK_SIZE + (FS_INODE_BITMAP_LENGTH * FS_BLOCK_SIZE) + (FS_DATA_BITMAP_LENGTH * FS_BLOCK_SIZE)
    
/* File System Structures */

typedef struct SuperBlock SuperBlock;
struct SuperBlock
{
    uint32_t magic_number; /* File system magic number */
    uint32_t blocks_count;  /* Number of blocks in fblocks_countile system */
    uint32_t inode_blocks; /* Number of blocks reserved for inodes */
    uint32_t inodes_count;  /* Number of inodes in file system */
    uint32_t free_data_blocks_count; /* value indicating the total number of free data blocks */
};

typedef struct Inode Inode;
struct Inode
{
    uint8_t type; // format of the file, see fs_filetype enumeration
    uint64_t size; // the size of the file in bytes
    uint64_t atime; // last time the file was accessed (UNIX time)
    uint64_t mtime; // last time the file was modified (UNIX time)
    uint32_t direct_pointers[FS_POINTERS_PER_INODE]; // block where the direct pointer, if the adress is 0x0000 that means no block is used on this pointer
    uint32_t indirect_pointers[FS_INDIRECT_POINTERS_PER_INODE]; //same but point to an indirect blocks           
};

enum fs_filetype {
    FS_TYPE_NONE = 0xFF, // deleted or non existant file, you can overwrite the inode
    FS_TYPE_FILE = 0xF1, // regular file
    FS_TYPE_LINK = 0x12, // symbolic link
    FS_TYPE_DIR  = 0xD1 // directory
};

typedef struct FileSystem FileSystem;
struct FileSystem
{
    Disk *disk;           /* Disk file system is mounted on */
    bool free_data[FS_BLOCK_SIZE*FS_INODE_BITMAP_LENGTH];    /* Free data bitmap */
    bool free_inodes[FS_BLOCK_SIZE*FS_DATA_BITMAP_LENGTH];    /* Inode bitmap */
    SuperBlock superblock; /* File system meta data */
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

/**
 * Allocate an Inode in the FileSystem Inode table by doing the following:
 *
 *  1. Search Inode table for free inode.
 *
 *  2. Reserve free inode in Inode table.
 *
 * Note: Be sure to record updates to Inode table to Disk.
 *
 * @param       fs      Pointer to FileSystem structure.
 * @return      Inode number of allocated Inode.
 **/
ssize_t fs_create(FileSystem *fs);

/**
 * Remove Inode and associated data from FileSystem by doing the following:
 *
 *  1. Load and check status of Inode.
 *
 *  2. Release any direct blocks.
 *
 *  3. Release any indirect blocks.
 *
 *  4. Mark Inode as free in Inode table.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Whether or not removing the specified Inode was successful.
 **/
bool fs_remove(FileSystem *fs, size_t inode_number);

/**
 * Return size of specified Inode.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Size of specified Inode (-1 if does not exist).
 **/
ssize_t fs_stat(FileSystem *fs, size_t inode_number);

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
ssize_t fs_read(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset);

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
ssize_t fs_write(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset);

#endif
