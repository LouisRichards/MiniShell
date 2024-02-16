/* fs.c: SimpleFS file system */

#include <stdio.h> //DEBUG
#include <string.h>
#include "fs/fs.h"
#include "fs/disk.h"
#include "utils.h"

/* Internal prototypes */
bool get_inode(FileSystem *fs, size_t inode_number, Inode *inode);
bool save_inode(FileSystem *fs, size_t inode_number, Inode *inode);
bool inode_operation(FileSystem *filesystem, size_t inode_number, Inode *inode, bool write);
bool get_superblock(FileSystem *fs, SuperBlock *superblock);
bool save_superblock(FileSystem *fs, SuperBlock *superblock);
bool superblock_operation(FileSystem *fs, SuperBlock *superblock, bool write);
bool check_inode_integrity(Inode *inode);
bool read_block(Disk *disk, unsigned char data[FS_BLOCK_SIZE], size_t block);
bool write_block(Disk *disk, unsigned char data[FS_BLOCK_SIZE], size_t block);
ssize_t allocate_free_block(FileSystem *fs);

/* External Functions */
void fs_debug(Disk *disk)
{
}

bool fs_format(Disk *disk, bool quick_format)
{
    if (disk_is_open(disk))
    {
        log_error("can't format, disk is open\n");
        return false;
    }

    return true;
}

bool fs_mount(FileSystem *fs, Disk *disk)
{
    return true;
}

void fs_unmount(FileSystem *fs)
{
}

ssize_t fs_create(FileSystem *fs)
{
    return 0;
}

bool fs_remove(FileSystem *fs, size_t inode_number)
{
    return true;
}

ssize_t fs_stat(FileSystem *fs, size_t inode_number)
{
    return 0;
}

ssize_t fs_read(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset)
{
    return 0;
}

ssize_t fs_write(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset)
{

    return 0;
}

/* Internal Functions */

// read inode inode_number in the inode table
bool get_inode(FileSystem *fs, size_t inode_number, Inode *inode)
{
    if (!inode_operation(fs, inode_number, inode, false))
    {
        log_error("[fs] Error loading the inode");
        return false;
    }
    return true;
}

bool save_inode(FileSystem *fs, size_t inode_number, Inode *inode)
{
    if (!inode_operation(fs, inode_number, inode, true))
    {
        log_error("[fs] Error loading the inode");
        return false;
    }
    return true;
}

// if write is false it will read the inode and put it in the inode struct, if write is true it will
// write the inode the function was given to the disk
bool inode_operation(FileSystem *filesystem, size_t inode_number, Inode *inode, bool write)
{
    int inode_addr;
    int ptr_offset = 0;
    uint8_t inode_buf[FS_INODE_SIZE] = {0};
    const int max_inodes = (FS_BLOCK_SIZE / FS_INODE_SIZE) * FS_INODE_BLOCKS;
    log_debug("max_inodes: %u, start address=%u\n", max_inodes, FS_INODE_TABLE_START_ADDR);

    // check if we are outside the inode table
    if ((inode_number + 1) > max_inodes)
    {
        log_error("[fs] trying to access an inode outside the inode table\n");
        return false;
    }
    inode_addr = FS_INODE_TABLE_START_ADDR + inode_number * FS_INODE_SIZE;
    log_debug("inode addr : %u\n", inode_addr);

    if (!write)
    {
        // load the inode into the buffer
        if (disk_read_raw(filesystem->disk, inode_buf, FS_INODE_SIZE, inode_addr) != DISK_SUCCESS)
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
            log_debug("writing dp: %04X\n", inode->direct_pointers[i]);
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

    if (write)
    {
        // write inode buffer to disk
        if (disk_write_raw(filesystem->disk, inode_buf, FS_INODE_SIZE, inode_addr) != DISK_SUCCESS)
        {
            log_error("[fs] cannot write inode to disk\n");
            return false;
        }
    }

    return true;
}

// return false if the inode is not properly set up
// also returns false if type is none
bool check_inode_integrity(Inode *inode)
{
    if (inode->type == FS_TYPE_NONE || inode->type != FS_TYPE_FILE || inode->type != FS_TYPE_LINK || inode->type != FS_TYPE_DIR)
    {
        return false;
    }

    // check if every pointers point to a valid data block

    // check if size is matching the number of data block

    return true;
}

bool get_superblock(FileSystem *fs, SuperBlock *superblock)
{
    if (!superblock_operation(fs, superblock, false))
    {
        log_error("[fs] Error getting superblock\n");
        return false;
    }

    return true;
}

bool save_superblock(FileSystem *fs, SuperBlock *superblock)
{
    if (!superblock_operation(fs, superblock, true))
    {
        log_error("[fs] Error saving superblock\n");
        return false;
    }

    return true;
}

bool superblock_operation(FileSystem *fs, SuperBlock *superblock, bool write)
{
    uint8_t superblock_buf[FS_BLOCK_SIZE] = {0};
    int ptr_offset = 0;
    if (!write)
    {
        if (!read_block(fs->disk, superblock_buf, 0))
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
        if (!write_block(fs->disk, superblock_buf, 0))
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
bool read_block(Disk *disk, unsigned char data[FS_BLOCK_SIZE], size_t block)
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
bool write_block(Disk *disk, unsigned char data[FS_BLOCK_SIZE], size_t block)
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
    FileSystem fs;
    SuperBlock superblock;
    Inode inode;
    Disk *disk = init_disk_struct();
    disk_open(disk, "data/test/fs/1.vdisk");
    fs.disk = disk;

    superblock.magic_number = FS_MAGIC_NUMBER;
    superblock.blocks_count = 258;
    superblock.inode_blocks = 255;
    superblock.inodes_count = 1;
    superblock.free_data_blocks_count = 240;
    save_superblock(&fs, &superblock);
    get_superblock(&fs, &superblock);
    log_debug("magic: %X, block count: %u, inode blocks: %u, inodes count: %u, free data blocks count: %u\n", superblock.magic_number, superblock.blocks_count, superblock.inode_blocks, superblock.inodes_count, superblock.free_data_blocks_count);
    get_inode(&fs, 0, &inode);
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

    save_inode(&fs, 1, &inode2);
    get_inode(&fs, 1, &inode2_read);

    disk_close(disk);
}
