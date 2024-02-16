/* fs.c: SimpleFS file system */

#include <stdio.h>  //DEBUG
#include <stdlib.h> //DEBUG
#include <fcntl.h>  //DEBUG
#include <unistd.h> //DEBUG
#include <time.h>
#include <string.h>
#include "fs/fs.h"
#include "fs/disk.h"
#include "utils.h"

/* Internal structs */
typedef struct Inode Inode;
struct Inode
{
    uint32_t number;                                            // not saved in the disk
    uint8_t type;                                               // format of the file, see fs_filetype enumeration
    uint64_t size;                                              // the size of the file in bytes
    uint64_t atime;                                             // last time the file was accessed (UNIX time)
    uint64_t mtime;                                             // last time the file was modified (UNIX time)
    uint32_t direct_pointers[FS_POINTERS_PER_INODE];            // block where the direct pointer, if the adress is 0x0000 that means no block is used on this pointer
    uint32_t indirect_pointers[FS_INDIRECT_POINTERS_PER_INODE]; // same but point to an indirect blocks
};

typedef struct Directory Directory;
struct Directory
{
    uint32_t inode;                       // number of the inode
    uint8_t name_len;                     // name length
    uint8_t file_type;                    // type of the inode, see fs_filetype enumeration
    uint8_t name[FS_MAX_FILENAME_LENGTH]; // name of the entry
};

typedef struct SuperBlock SuperBlock;
struct SuperBlock
{
    uint32_t magic_number;           /* File system magic number */
    uint32_t blocks_count;           /* Number of blocks in fblocks_countile system */
    uint32_t inode_blocks;           /* Number of blocks reserved for inodes */
    uint32_t inodes_count;           /* Number of inodes in file system */
    uint32_t free_data_blocks_count; /* value indicating the total number of free data blocks */
};

enum BlockOperation
{
    FS_BLOCK_WRITE,
    FS_BLOCK_READ,
    FS_BLOCK_REMOVE
};

/* Internal prototypes */
bool inode_io_operation(Disk *disk, Inode *inode, uint8_t *data, size_t length, size_t offset, bool write);
bool inode_block_operation(Disk *disk, Inode *inode, size_t block, uint8_t data[FS_BLOCK_SIZE], int operation_type);
bool get_inode_block(Disk *disk, Inode *inode, size_t block, uint8_t data[FS_BLOCK_SIZE]);
bool save_inode_block(Disk *disk, Inode *inode, size_t block, uint8_t data[FS_BLOCK_SIZE]);
bool get_inode(Disk *disk, size_t inode_number, Inode *inode);
bool save_inode(Disk *disk, size_t inode_number, Inode *inode);
ssize_t allocate_ptr_block(Disk *disk, bool inode);
bool inode_operation(Disk *disk, size_t inode_number, Inode *inode, bool write);
bool get_superblock(Disk *disk, SuperBlock *superblock);
bool save_superblock(Disk *disk, SuperBlock *superblock);
bool superblock_operation(Disk *disk, SuperBlock *superblock, bool write);
bool check_inode_integrity(Inode *inode);
bool read_block(Disk *disk, uint8_t data[FS_BLOCK_SIZE], size_t block);
bool write_block(Disk *disk, uint8_t data[FS_BLOCK_SIZE], size_t block);
bool unlink_inode(Disk *disk, size_t inode_number);
bool inode_bitmap_get(Disk *disk, size_t inode);
ssize_t allocate_free_block(FileSystem *fs);

/* External Functions */

void fs_debug(Disk *disk)
{
}

bool fs_format(Disk *disk, bool quick_format)
{
    // make root "/"
    return true;
}

bool fs_mount(FileSystem *fs, Disk *disk)
{
    fs->disk = disk;
    return true;
}

void fs_unmount(FileSystem *fs)
{
}

inode_t fs_create_file(FileSystem *fs, char *path)
{

    return 0;
}

inode_t fs_mkdir(FileSystem *fs, char *path)
{
    log_error("not implemented\n");
    return 0;
}

bool fs_rmdir(FileSystem *fs, inode_t dir)
{
    log_error("not implemented\n");
    return true;
}

bool fs_unlink(FileSystem *fs, inode_t file)
{
    if (!unlink_inode(fs->disk, file))
    {
        log_error("[fs] Cannot unlink\n");
        return false;
    }
    return true;
}

ssize_t fs_read(FileSystem *fs, size_t inode_number, uint8_t *data, size_t length, size_t offset)
{
    return 0;
}

ssize_t fs_write(FileSystem *fs, size_t inode_number, uint8_t *data, size_t length, size_t offset)
{
    return 0;
}

ssize_t fs_rename(char *oldpath, char *newpath)
{
    return 0;
}

bool fs_truncate(FileSystem *fs, inode_t file, size_t newsize)
{
    return true;
}

char *fs_readdir(FileSystem *fs, inode_t dir)
{ // retourne des chemins absolus
    return NULL;
}

inode_t fs_getinode(FileSystem *fs, char *path)
{
    return true;
}

InodeInfo fs_getinfo(FileSystem *fs, inode_t inode)
{
    InodeInfo sa = {0};
    return sa;
}

/* Internal Functions */

// write/read directory structure at pointer (on the disk)
bool directory_operation(Disk *disk, Directory *dir, uint32_t ptr, bool write)
{

    if (write)
    {
        if (disk_write_raw(disk, dir, FS_DIR_SIZE, ptr) != DISK_SUCCESS)
        {
            log_error("Cannot write folder to the disk");
        }
    }
    else
    {
        if (disk_read_raw(disk, dir, FS_DIR_SIZE, ptr) != DISK_SUCCESS)
        {
            log_error("Cannot read folder on the disk");
        }
    }

    return true;
}

bool create_root_folder(Disk *disk)
{
    // root folder must be inode 0
    if (!inode_bitmap_get(disk, 0))
    {
        log_error("[fs] Inode 0 is not root folder");
    }

    Inode root = {0};
    root.number = 0;
    root.type = FS_TYPE_DIR;

    if (!save_inode(disk, 0, &root))
    {
        return false;
    }
}

// read/write the bit parameter at a position
bool bitmap_operation(Disk *disk, bool inode_bitmap, size_t bit_pos, bool write, bool *bit)
{
    size_t bitmap_length = FS_DATA_BITMAP_LENGTH;
    size_t bitmap_addr = FS_DATA_BITMAP_START_ADDR;
    if (inode_bitmap)
    {
        bitmap_length = FS_INODE_BITMAP_LENGTH;
        bitmap_addr = FS_INODE_BITMAP_START_ADDR;
    }

    uint8_t data_bitmap[FS_BLOCK_SIZE * bitmap_length];
    if (disk_read_raw(disk, data_bitmap, FS_BLOCK_SIZE * bitmap_length, bitmap_addr) != DISK_SUCCESS)
    {
        log_error("[fs] Cannot read bitmap"); // TODO proper error handling
        return false;
    }

    size_t byte_pos = bit_pos / 8;
    if (byte_pos > bitmap_length * FS_BLOCK_SIZE)
    {
        log_error("[fs] trying to read outside bitmap\n");
        return false;
    }

    size_t offset = bit_pos % 8;
    uint8_t byte = data_bitmap[byte_pos];
    if (write)
    {
        if (*bit)
            BIT_SET(byte, offset);
        else
            BIT_CLEAR(byte, offset);

        data_bitmap[byte_pos] = byte;

        if (disk_write_raw(disk, data_bitmap, FS_BLOCK_SIZE * bitmap_length, bitmap_addr) != DISK_SUCCESS)
        {
            log_error("[fs] cannot write bitmap");
            return false;
            ;
        }
    }
    else
    {
        *bit = BIT_TEST(byte, offset);
    }

    return true;
}

// get bit of inode n in inode bitmap
bool inode_bitmap_get(Disk *disk, size_t inode)
{
    bool bit;
    if (!bitmap_operation(disk, true, inode, false, &bit))
    {
        exit(1); // TODO remove exit and use standard error handling
    }
    return bit;
}

bool inode_bitmap_set(Disk *disk, size_t inode, bool bit)
{
    if (!bitmap_operation(disk, true, inode, true, &bit))
    {
        exit(1);
    }
    return true;
}

// get bit of block n in inode bitmap
bool data_bitmap_get(Disk *disk, size_t block)
{
    bool bit;
    if (!bitmap_operation(disk, false, block, false, &bit))
    {
        exit(1);
    }
    return bit;
}

bool data_bitmap_set(Disk *disk, size_t block, bool bit)
{
    log_debug("[data bitmap] removing %u\n", block);
    if (!bitmap_operation(disk, false, block, true, &bit))
    {
        exit(1);
    }
    return true;
}

bool unlink_inode(Disk *disk, size_t inode_number)
{
    const size_t data_addr = FS_DATA_START_ADDR; // TODO remove all defines and replace with variable constant to prevent things like this
    Inode inode;
    if (!get_inode(disk, inode_number, &inode))
    {
        log_error("[fs] Cannot find inode (%lu) to delete", inode_number);
        return false;
    }

    // unallocate inode data on bitmap
    for (int i = 0; i < FS_POINTERS_PER_INODE; i++)
    {
        uint32_t block_ptr = inode.direct_pointers[i];
        if (block_ptr != 0x00)
        {
            size_t block_nbr = (block_ptr - data_addr) / FS_BLOCK_SIZE;
            log_debug("direct blk=%lu, start_addr=%lu, calc=%u\n", block_ptr, data_addr, block_nbr);
            data_bitmap_set(disk, block_nbr, 0);
        }
    }

    for (int i = 0; i < FS_INDIRECT_POINTERS_PER_INODE; i++)
    {
        size_t ind_block_ptr = inode.indirect_pointers[i];
        uint8_t ind_block[FS_BLOCK_SIZE];
        if (ind_block_ptr != 0x00)
        {
            if (disk_read_raw(disk, ind_block, FS_BLOCK_SIZE, ind_block_ptr) != DISK_SUCCESS)
            {
                return false;
            }

            size_t ptr_offset = 0;
            for (int y = 0; y < FS_POINTER_PER_BLOCK; y++)
            {
                uint32_t block_ptr = get_uint32(ind_block + ptr_offset);
                if (block_ptr != 0x00)
                {
                    size_t block_nbr = (block_ptr - data_addr) / FS_BLOCK_SIZE;
                    log_debug("INdirect blk=%lu, start_addr=%lu, calc=%u\n", block_ptr, data_addr, block_nbr);
                    data_bitmap_set(disk, block_nbr, 0);
                }

                ptr_offset += FS_POINTER_SIZE;
            }
        }
    }

    // uncallocate inode on bitmap
    inode_bitmap_set(disk, inode_number, 0);

    // wipe inode
    uint8_t null_inode[FS_INODE_SIZE] = {0};
    log_debug("wiping inode %lu\n", FS_INODE_TABLE_START_ADDR + (inode_number * FS_INODE_SIZE));
    if (disk_write_raw(disk, null_inode, FS_INODE_SIZE, FS_INODE_TABLE_START_ADDR + (inode_number * FS_INODE_SIZE)) != DISK_SUCCESS)
    {
        return false;
    }

    return true;
}

// read inode inode_number in the inode table
bool get_inode(Disk *disk, size_t inode_number, Inode *inode)
{
    if (!inode_operation(disk, inode_number, inode, false))
    {
        log_error("[fs] Error loading the inode\n");
        return false;
    }
    return true;
}

bool save_inode(Disk *disk, size_t inode_number, Inode *inode)
{
    if (!inode_operation(disk, inode_number, inode, true))
    {
        log_error("[fs] Error loading the inode\n");
        return false;
    }
    return true;
}

// write/read data to/from file at specified offset
bool inode_io_operation(Disk *disk, Inode *inode, uint8_t *data, size_t length, size_t offset, bool write)
{
    log_debug("%u\n", write);
    size_t size = offset + length;

    /* check if inode is valid */
    if ((inode->number < FS_MAX_INODES) && (!inode_bitmap_get(disk, inode->number)))
    {
        log_error("[fs] Trying to access a non existent inode\n");
        return false;
    }

    size_t inode_size = inode->size;
    if (!check_inode_integrity(inode))
    {
        log_error("[fs] The inode (%u) is corrupted\n", inode->number);
        return false;
    }

    if (size >= FS_MAX_FILE_SIZE)
    {
        log_error("[fs] Trying to read/write more than the maximum file size\n");
        return false;
    }

    if (!write && size > inode_size)
    {
        log_error("[fs] Trying to read more than the file size\n!!");
        return false;
    }

    /* read/write each blocks */
    size_t data_offset = 0;
    size_t first_block = offset / FS_BLOCK_SIZE;
    size_t starting_point = offset - (first_block * FS_BLOCK_SIZE); // only useful for reading the first block, set to 0 afterward
    size_t block_io_size = FS_BLOCK_SIZE - starting_point;
    size_t blocks_nbr = math_ceil(length / ((double)FS_BLOCK_SIZE)); // how many blocks we're gonna read/write

    log_debug("inode_io_operation first block_nbr=%lu, blocks=%lu", first_block, blocks_nbr);
    uint8_t block_data[FS_BLOCK_SIZE] = {0};
    for (int block = first_block; block < (first_block + blocks_nbr); block++)
    {

        if (block == blocks_nbr - 1)
        { // if we're at the last block maybe we may not need to r/w the entire block
            block_io_size = size - (block * FS_BLOCK_SIZE);
        }

        // if block doesn't exist create it
        if (write && block >= inode_size / FS_BLOCK_SIZE)
        {
            uint8_t zerobuf[FS_BLOCK_SIZE] = {0};
            log_debug("must create block %d\n", block);
            if (!save_inode_block(disk, inode, block, zerobuf))
            {
                log_error("[fs] cannot create block\n");
                return false;
            }
        }

        if (!get_inode_block(disk, inode, block, block_data))
        {
            return false;
        }

        if (write)
        {
            memcpy(block_data + starting_point, data + data_offset, block_io_size);
            if (!save_inode_block(disk, inode, block, block_data))
            {
                return false;
            }
        }
        else
        {
            // log_debug("[reading io] block: %s\n", block_data+starting_point); //DEBUG
            memcpy(data + data_offset, block_data + starting_point, block_io_size);
        }

        data_offset += block_io_size;
        block_io_size = FS_BLOCK_SIZE;
        starting_point = 0;
    }

    // update inode size
    inode_size += length;
    inode->size = inode_size;

    return true;
}

bool get_inode_block(Disk *disk, Inode *inode, size_t block, uint8_t data[FS_BLOCK_SIZE])
{
    if (!inode_block_operation(disk, inode, block, data, FS_BLOCK_READ))
    {
        log_error("[fs] Cannot read inode block %lu of inode %lu\n", block, inode->number);
        return false;
    }

    return true;
}

bool save_inode_block(Disk *disk, Inode *inode, size_t block, uint8_t data[FS_BLOCK_SIZE])
{
    if (!inode_block_operation(disk, inode, block, data, FS_BLOCK_WRITE))
    {
        log_error("[fs] Cannot write inode block %lu of inode %lu\n", block, inode->number);
        return false;
    }

    return true;
}

// retun address of block allocated, -1 if no more block/error
// if inode is true it allocate inode table slot, else it allocate data block
// also it will wipe the data of the block with 0x00 by default (change const if you don't want this)
ssize_t allocate_ptr_block(Disk *disk, bool inode)
{
    const bool wipe_block = true;
    int ptr = 0x0;
    // Find a block to allocate in data bitmap and set it
    bool found = false;
    size_t i = 0;
    while (!found)
    {
        bool bit;

        if (!bitmap_operation(disk, inode, i, false, &bit))
        {
            return -1;
        }

        if (!bit)
        {
            ptr = inode ? FS_INODE_TABLE_START_ADDR + (i * FS_INODE_SIZE) : FS_DATA_START_ADDR + (i * FS_BLOCK_SIZE);
            log_debug("free block n°%d found, addr=%d\n", i, ptr);
            // check if disk is big enought to address this block
            if (ptr >= disk_get_size(disk))
            {
                log_error("[fs] Disk is not big enought to allocate this block\n");
                return -1;
            }

            // write the bitmap
            bit = 1;
            if (!bitmap_operation(disk, inode, i, true, &bit))
            {
                return -1;
            }

            // wipe the block
            if (wipe_block)
            {
                uint8_t empty_block[FS_BLOCK_SIZE] = {0};
                if (disk_write_raw(disk, empty_block, FS_BLOCK_SIZE, ptr) != DISK_SUCCESS)
                {
                    log_error("[fs] cannot wipe new block\n");
                    return false;
                }
            }

            return ptr;
        }
        i++;
    }
    log_warning("[fs] No more blocks left to allocate");
    return -1;
}

// read/write/delete a block in the inode, starts from block 0
bool inode_block_operation(Disk *disk, Inode *inode, size_t block, uint8_t data[FS_BLOCK_SIZE], int operation_type)
{
    const size_t fs_pointers_per_inode = FS_POINTERS_PER_INODE;
    const size_t fs_pointers_per_block = FS_POINTER_PER_BLOCK;
    log_debug("Operation on inode block %lu, op_type=%d\n", block, operation_type);
    if (block > FS_MAX_FILE_SIZE / FS_BLOCK_SIZE)
    {
        log_error("[fs] Error, trying to get a block outside of the limits\n");
        return false;
    }

    ssize_t block_ptr = 0x0; // address of the block on the disk
    bool is_indirect_ptr = false;

    // find where the block is
    if (block < FS_POINTERS_PER_INODE)
    { // it's a direct pointer
        block_ptr = inode->direct_pointers[block];
    }
    else
    { // it's an indirect pointer
        is_indirect_ptr = true;
        block_ptr = inode->indirect_pointers[block / fs_pointers_per_block];
    }

    if (block_ptr == 0x0 && operation_type == FS_BLOCK_READ)
    { // error: reading unallocated block
        log_error("[fs] trying to read unallocated block\n");
        return false;
    }
    else if (block_ptr == 0x0 && operation_type == FS_BLOCK_WRITE && !is_indirect_ptr)
    { // allocating new direct block
        block_ptr = allocate_ptr_block(disk, false);
        log_debug("allocating a new direct block, at %u\n", block_ptr);
        if (block_ptr == -1)
        {
            log_error("cannot allocate new block\n");
            return false;
        }
        inode->direct_pointers[block] = block_ptr;
    }
    else if (is_indirect_ptr)
    { // reading/writing/creating block with indirection
        size_t ind_block = block / (fs_pointers_per_block);
        uint8_t ind_block_data[FS_BLOCK_SIZE] = {0};
        // size_t ind_block_offset = block - FS_POINTERS_PER_INODE - (ind_block * FS_POINTER_PER_BLOCK);
        size_t ind_block_offset = (block - fs_pointers_per_inode) % (fs_pointers_per_block);
        /// create indirect block if needed
        if (block_ptr == 0x00 && operation_type == FS_BLOCK_WRITE)
        {
            ssize_t new_ind_ptr = allocate_ptr_block(disk, false);
            if (new_ind_ptr == -1)
            {
                log_error("[fs] cannot allocate indirect ptr block\n");
                return false;
            }
            log_debug("creating new pointer block... ptr=%u\n", new_ind_ptr);
            inode->indirect_pointers[ind_block] = new_ind_ptr;
        }

        log_debug("ind_block=%lu (ptr=%lu), ind_block_offset=%lu\n", ind_block, inode->indirect_pointers[ind_block], ind_block_offset);
        if (disk_read_raw(disk, ind_block_data, FS_BLOCK_SIZE, inode->indirect_pointers[ind_block]) != DISK_SUCCESS)
        {
            log_error("[fs] Cannot read/ind_block indirect pointer\n");
            return false;
        }

        // get the pointer from the pointer block
        block_ptr = get_uint32(ind_block_data + (ind_block_offset * FS_POINTER_SIZE));
        if (block_ptr == 0x0 && operation_type == FS_BLOCK_READ)
        {
            log_error("[fs] reading NULL indirect pointer\n");
            return false;
        }
        else if (block_ptr == 0x0 && operation_type == FS_BLOCK_WRITE)
        {
            log_debug("allocating new block inside ptr block\n");
            block_ptr = allocate_ptr_block(disk, false);
            if (block_ptr == -1)
            {
                log_error("[fs] cannot allocate new block into ind block");
                return false;
            }
            uint8_t ptr_to_write[FS_POINTER_SIZE];
            write_uint32_arr(ptr_to_write, block_ptr); // TODO make this work with other ptr sizes
            log_debug("writing ptr to block: ...");
            if (disk_write_raw(disk, ptr_to_write, FS_POINTER_SIZE, inode->indirect_pointers[ind_block] + (ind_block_offset * FS_POINTER_SIZE)) != DISK_SUCCESS)
            {
                log_error("[fs] Cannot write ptr inside indirect block\n");
                return false;
            }
        }
    }

    // read/write/delete the block
    log_debug("block ptr=%lu\n", block_ptr);
    switch (operation_type)
    {
    case FS_BLOCK_WRITE:
        if (disk_write_raw(disk, data, FS_BLOCK_SIZE, block_ptr) != DISK_SUCCESS)
        {
            return false;
        }
        break;
    case FS_BLOCK_READ:
        if (disk_read_raw(disk, data, FS_BLOCK_SIZE, block_ptr) != DISK_SUCCESS)
        {
            return false;
        }
    }

    return true;
}

// if write is false it will read the inode and put it in the inode struct, if write is true it will
// write the inode struct to the disk
bool inode_operation(Disk *disk, size_t inode_number, Inode *inode, bool write)
{

    int inode_addr;
    int ptr_offset = 0;
    uint8_t inode_buf[FS_INODE_SIZE] = {0};
    inode->mtime = (uint64_t)time(NULL);

    // check if we are outside the inode table
    if ((inode_number + 1) > FS_MAX_INODES)
    {
        log_error("[fs] trying to access an inode outside the inode table\n");
        return false;
    }
    // if we're tring to read an inode it must exist on the bitmap
    if (!write && !inode_bitmap_get(disk, inode_number))
    {
        log_error("[fs] Inode doesn't exist on the bitmap\n");
        return false;
    }

    // DEBUG
    if (write && inode_bitmap_get(disk, inode_number))
    {
        log_debug("[fs] overwriting another inode\n");
    }

    inode_addr = FS_INODE_TABLE_START_ADDR + inode_number * FS_INODE_SIZE;
    log_debug("inode addr : %u\n", inode_addr);

    if (!write)
    {
        // load the inode into the buffer
        if (disk_read_raw(disk, inode_buf, FS_INODE_SIZE, inode_addr) != DISK_SUCCESS)
        {
            log_error("[fs] cannot load inode from disk\n");
            return false;
        }
    }

    if (write)
        inode_buf[0] = inode->type;
    else
        inode->type = inode_buf[0];

    ptr_offset += 1;

    if (write)
        write_uint64_arr(inode_buf + ptr_offset, inode->size);
    else
        inode->size = get_uint64(inode_buf + ptr_offset);

    ptr_offset += 8;

    if (write)
        write_uint64_arr(inode_buf + ptr_offset, inode->atime);
    else
        inode->atime = get_uint64(inode_buf + ptr_offset);

    ptr_offset += 8;

    if (write)
        write_uint64_arr(inode_buf + ptr_offset, inode->mtime);
    else
        inode->mtime = get_uint64(inode_buf + ptr_offset);

    ptr_offset += 8;
    for (int i = 0; i < FS_POINTERS_PER_INODE; i++)
    {
        if (write)
        {
            write_uint32_arr(inode_buf + ptr_offset, inode->direct_pointers[i]);
        }
        else
            inode->direct_pointers[i] = get_uint32(inode_buf + ptr_offset);
        ptr_offset += 4;
    }
    for (int i = 0; i < FS_INDIRECT_POINTERS_PER_INODE; i++)
    {
        if (write)
            write_uint32_arr(inode_buf + ptr_offset, inode->indirect_pointers[i]);
        else
            inode->indirect_pointers[i] = get_uint32(inode_buf + ptr_offset);
        ptr_offset += 4;
    }

    if (!write)
    {
        inode->number = inode_number;
    }

    if (write)
    {
        // write inode buffer to disk
        if (disk_write_raw(disk, inode_buf, FS_INODE_SIZE, inode_addr) != DISK_SUCCESS)
        {
            log_error("[fs] cannot write inode to disk\n");
            return false;
        }

        // write inode bitmap
        inode_bitmap_set(disk, inode_number, true);
    }

    return true;
}

// return false if the inode is not properly set up
// also returns false if type is none
bool check_inode_integrity(Inode *inode)
{
    //~ if (inode->type != FS_TYPE_NONE || inode->type != FS_TYPE_FILE || inode->type != FS_TYPE_LINK || inode->type != FS_TYPE_DIR) {
    //~ return false;
    //~ }

    // check if every pointers point to a valid data block

    // check if size is matching the number of data block

    return true;
}

bool get_superblock(Disk *disk, SuperBlock *superblock)
{
    if (!superblock_operation(disk, superblock, false))
    {
        log_error("[fs] Error getting superblock\n");
        return false;
    }

    return true;
}

bool save_superblock(Disk *disk, SuperBlock *superblock)
{
    if (!superblock_operation(disk, superblock, true))
    {
        log_error("[fs] Error saving superblock\n");
        return false;
    }

    return true;
}

bool superblock_operation(Disk *disk, SuperBlock *superblock, bool write)
{
    uint8_t superblock_buf[FS_BLOCK_SIZE] = {0};
    int ptr_offset = 0;
    if (!write)
    {
        if (!read_block(disk, superblock_buf, 0))
        {
            log_error("[fs] cannot read superblock from disk\n");
            return false;
        }
    }

    if (write)
        write_uint32_arr(superblock_buf, superblock->magic_number);
    else
        superblock->magic_number = get_uint32(superblock_buf);
    ptr_offset += 4;

    if (write)
        write_uint32_arr(superblock_buf + ptr_offset, superblock->blocks_count);
    else
        superblock->blocks_count = get_uint32(superblock_buf + ptr_offset);
    ptr_offset += 4;

    if (write)
        write_uint32_arr(superblock_buf + ptr_offset, superblock->inode_blocks);
    else
        superblock->inode_blocks = get_uint32(superblock_buf + ptr_offset);
    ptr_offset += 4;

    if (write)
        write_uint32_arr(superblock_buf + ptr_offset, superblock->inodes_count);
    else
        superblock->inodes_count = get_uint32(superblock_buf + ptr_offset);
    ptr_offset += 4;

    if (write)
        write_uint32_arr(superblock_buf + ptr_offset, superblock->free_data_blocks_count);
    else
        superblock->free_data_blocks_count = get_uint32(superblock_buf + ptr_offset);

    if (write)
    {
        if (!write_block(disk, superblock_buf, 0))
        {
            log_error("[fs] cannot write superblock to disk\n");
            return false;
        }
    }
    return true;
}

void initialize_free_block_bitmap(FileSystem *fs)
{
}

/*
    read a specific block and fill data buffer with it

    this is an important function because it allows the filesystem to use a differrent block size
    than the disk sector size for example if disk sector is 512 and fs block is 4096 it
    will return a block composed of 8 disk sector

*/
bool read_block(Disk *disk, uint8_t data[FS_BLOCK_SIZE], size_t block)
{
    int pos = block * FS_BLOCK_SIZE;

    if ((block * FS_BLOCK_SIZE) > disk_get_size(disk))
    {
        log_error("[fs] trying to read fs block outside the disk\n");
        return false;
    }

    if (disk_read_raw(disk, data, FS_BLOCK_SIZE, pos) != DISK_SUCCESS)
    {
        log_error("[fs] cannot read the disk\n");
        return false;
    }

    return true;
}

/* function similarly to read block */
bool write_block(Disk *disk, uint8_t data[FS_BLOCK_SIZE], size_t block)
{
    int pos = block * FS_BLOCK_SIZE;

    if ((block * FS_BLOCK_SIZE) > disk_get_size(disk))
    {
        log_error("[fs] trying to read fs block outside the disk\n");
        return false;
    }

    if (disk_write_raw(disk, data, FS_BLOCK_SIZE, pos) != DISK_SUCCESS)
    {
        log_error("[fs] cannot write the disk\n");
        return false;
    }

    return true;
}

ssize_t allocate_free_block(FileSystem *fs)
{
    return 0;
}

int main(void)
{
    Disk *disk = init_disk_struct();
    disk_open(disk, "data/test/fs/2.vdisk");

    Directory dir;
    Directory dir_r;
    dir.file_type = FS_TYPE_FILE;
    dir.inode = 666;
    dir.name_len = 5;
    strcpy(dir.name, "travail (oui)");

    directory_operation(disk, &dir, 0x0048B000, true);
    directory_operation(disk, &dir_r, 0x0048B000, false);

    return 0;
}

// other mains I used
/*
int main(void) {
    Disk *disk = init_disk_struct();
    disk_create("data/test/fs/2.vdisk", 2048);
    disk_open(disk, "data/test/fs/2.vdisk");
    Inode i = {0};
    i.type = FS_TYPE_FILE;
    Inode i2 = {0};
    i2.type = FS_TYPE_FILE;
    save_inode(disk, 1, &i);
    save_inode(disk, 4, &i2);
    get_inode(disk, 1, &i);
    get_inode(disk, 4, &i2);

    int article_fd = open("hello.txt", O_RDWR);
    int article_moi_fd = open("hello_moi.txt", O_CREAT | O_RDWR, 0777);
    size_t article_s = 25611;
    uint8_t *data = malloc(sizeof(uint8_t) * article_s);
    uint8_t *data_r = malloc(sizeof(uint8_t) * article_s);
    read(article_fd, data, article_s);
    //memset(data, 0x14, FS_BLOCK_SIZE*20);
    inode_io_operation(disk, &i, data, article_s, 0, true);
    log_debug("size ::: %lu\n", i.size);
    save_inode(disk, 4, &i);

    inode_io_operation(disk, &i2, data, article_s, 0, true);
    save_inode(disk, 1, &i2);

    inode_io_operation(disk, &i, data_r, article_s, 0, false);
    write(article_moi_fd, data_r, article_s);
    get_inode(disk, 1, &i);
    unlink_inode(disk, 1);

    free(data);
    free(data_r);
    disk_close(disk);
}
*/
/*
int main(void) {
    SuperBlock superblock;
    Inode inode;
    Disk *disk = init_disk_struct();
    disk_open(disk, "data/test/fs/1.vdisk");

    superblock.magic_number = FS_MAGIC_NUMBER;
    superblock.blocks_count = 258;
    superblock.inode_blocks = 255;
    superblock.inodes_count = 1;
    superblock.free_data_blocks_count = 240;
    save_superblock(disk, &superblock);
    get_superblock(disk, &superblock);
    log_debug("magic: %X, block count: %u, inode blocks: %u, inodes count: %u, free data blocks count: %u\n", superblock.magic_number, superblock.blocks_count, superblock.inode_blocks, superblock.inodes_count, superblock.free_data_blocks_count);
    get_inode(disk, 0, &inode);
    log_debug("type : %u, size= %lu, atime=%lu, mtime=%lu\n", inode.type, inode.size, inode.atime, inode.mtime);
    for (int i = 0; i < FS_POINTERS_PER_INODE; i++)
    {
        log_debug("pointer %d: %u\n", i, inode.direct_pointers[i]);
    }
    for (int i = 0; i < FS_INDIRECT_POINTERS_PER_INODE; i++)
    {
        log_debug("indirect pointer %d: %u\n", i, inode.indirect_pointers[i]);
    }


    Inode inode2;
    Inode inode2_read;
    inode2.type = FS_TYPE_FILE;
    inode2.size = 0x1011121314151617;
    inode2.atime = 0x3258961548320985;
    inode2.mtime = 0x2203807960321548;
    inode2.direct_pointers[0] = 0x80000001;
    inode2.direct_pointers[1] = 0x70000002;
    inode2.direct_pointers[2] = 0x60000003;
    inode2.direct_pointers[3] = 0x50000004;
    inode2.direct_pointers[4] = 0x40000005;
    inode2.direct_pointers[5] = 0x30000006;
    inode2.direct_pointers[6] = 0x20000007;
    inode2.direct_pointers[7] = 0x10000008;
    inode2.indirect_pointers[0] = 0x12345678;
    inode2.indirect_pointers[1] = 0x87654321;
    inode2.indirect_pointers[2] = 0x13579001;
    inode2.indirect_pointers[3] = 0x08641368;

    save_inode(disk, 1, &inode2);
    save_inode(disk, 6, &inode2);
    save_inode(disk, 0, &inode2);
    get_inode(disk, 1, &inode2_read);

    uint8_t data[FS_BLOCK_SIZE] = {0};
    //inode_block_operation(disk, 0, 0, data, FS_BLOCK_WRITE);
    log_debug("--------------------------- allocating block\n");
    //allocate_ptr_block(disk, false);
    disk_close(disk);
}*/
