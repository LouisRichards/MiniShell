/* fs.c: SimpleFS file system */

#include <stdio.h> //DEBUG
#include <string.h>
#include "fs/fs.h"
#include "fs/disk.h"
#include "utils.h"

/* Internal prototypes */
bool inode_io_operation(FileSystem* fs, size_t inode_number, uint8_t* data, size_t length, size_t offset, bool write);
bool inode_block_operation(Disk* disk, size_t inode_number, size_t block, uint8_t data[FS_BLOCK_SIZE], bool write);
bool get_inode_block(Disk* disk, size_t inode_number, size_t block, uint8_t data[FS_BLOCK_SIZE]);
bool save_inode_block(Disk* disk, size_t inode_number, size_t block, uint8_t data[FS_BLOCK_SIZE]);
bool get_inode(Disk *disk, size_t inode_number, Inode *inode);
bool save_inode(Disk *disk, size_t inode_number, Inode *inode);
int allocate_ptr_block(Disk* disk);
bool inode_operation(Disk* disk, size_t inode_number, Inode* inode, bool write);
bool get_superblock(Disk *disk, SuperBlock *superblock);
bool save_superblock(Disk *disk, SuperBlock *superblock);
bool superblock_operation(Disk *disk, SuperBlock *superblock, bool write);
bool check_inode_integrity(Inode* inode);
bool read_block(Disk* disk, uint8_t data[FS_BLOCK_SIZE], size_t block);
bool write_block(Disk* disk, uint8_t data[FS_BLOCK_SIZE], size_t block);
ssize_t allocate_free_block(FileSystem *fs);

/* External Functions */
void fs_debug(Disk *disk)
{
    
}


bool fs_format(Disk *disk, bool quick_format)
{
    if (disk_is_open(disk)) {
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

ssize_t fs_read(FileSystem *fs, size_t inode_number, uint8_t *data, size_t length, size_t offset)
{
    return 0;
}

ssize_t fs_write(FileSystem *fs, size_t inode_number, uint8_t *data, size_t length, size_t offset)
{
    
    
    return 0;
}

/* Internal Functions */
bool init_inode(Disk* disk, Inode *inode) {
    
    return true;
}

// read inode inode_number in the inode table
bool get_inode(Disk *disk, size_t inode_number, Inode *inode) {
    if (!inode_operation(disk, inode_number, inode, false)) {
    	log_error("[fs] Error loading the inode");
    	return false;
    }
    return true;
}

bool save_inode(Disk* disk, size_t inode_number, Inode *inode) {
    if (!inode_operation(disk, inode_number, inode, true)) {
    	log_error("[fs] Error loading the inode");
    	return false;
    }
    return true;
}


// write/read data to/from file at specified offset
bool inode_io_operation(FileSystem* fs, size_t inode_number, uint8_t* data, size_t length, size_t offset, bool write) {
    size_t size = offset + length;
    /* check if inode is valid */
    if ((inode_number < FS_MAX_INODES) && (!fs->data_bitmap[inode_number])) {
        log_error("[fs] Trying to access a non existent inode\n");
        return false;
    }
    Inode inode;
    get_inode(fs->disk, inode_number, &inode);
    
    if (!check_inode_integrity(&inode)) {
        log_error("[fs] The inode (%u) is corrupted\n", inode_number);
        return false;
    }
    
    if (size >= FS_MAX_FILE_SIZE) {
        log_error("[fs] Trying to read/write more than the maximum file size\n");
        return false;
    }

    if (!write && size >= inode.size) {
        log_error("[fs] Trying to read more than the file size");
        return false;
    }
    
    /* read/write each blocks */
    size_t data_offset = 0;
    size_t first_block = offset / FS_BLOCK_SIZE;
    size_t starting_point = offset-(first_block * FS_BLOCK_SIZE); //only useful for reaing the first block, set to 0 afterward
    size_t block_io_size = FS_BLOCK_SIZE-starting_point;
    size_t blocks_nbr = math_ceil(length / ((double)FS_BLOCK_SIZE)); // how many blocks we're gonna read/write
    log_debug("inode_io_operation first block_nbr=%lu, blocks=%lu", first_block, blocks_nbr);
    uint8_t block_data[FS_BLOCK_SIZE] = {0};
    for (int block = first_block; block < (first_block+blocks_nbr); block++) {
        
        if (block == blocks_nbr-1) { //if we're at the last block maybe we may not need to r/w the entire block
            block_io_size = size - (block * FS_BLOCK_SIZE);
        }
        
        if (!get_inode_block(fs->disk, inode_number, block, block_data)) {
            return false;
        }
        
        if (write) {
            memcpy(block_data+starting_point, data+data_offset, block_io_size);
            if (!save_inode_block(fs->disk, inode_number, block, block_data)) {
                return false;
            }
        } else {
            memcpy(data+data_offset, block_data+starting_point, block_io_size);
        }

        
        data_offset += block_io_size;
        block_io_size = FS_BLOCK_SIZE;
        starting_point = 0;
    }
    return true;
}


bool get_inode_block(Disk* disk, size_t inode_number, size_t block, uint8_t data[FS_BLOCK_SIZE]) {
    if (!inode_block_operation(disk, inode_number, block, data, 0)) {
        log_error("[fs] Cannot read inode block %lu of inode %lu", block, inode_number);
        return false;
    }

    return true;
}

bool save_inode_block(Disk* disk, size_t inode_number, size_t block, uint8_t data[FS_BLOCK_SIZE]) {
    if (!inode_block_operation(disk, inode_number, block, data, 1)) {
        log_error("[fs] Cannot write inode block %lu of inode %lu", block, inode_number);
        return false;
    }

    return true;
}

//retun address of data block allocated, -1 if no more block/error 
int allocate_ptr_block(Disk* disk) {
    int ptr = 0x0;
    // Find a block to allocate in data bitmap and mark it
    SuperBlock superblock;
    get_superblock(disk, &superblock);
    for (int i = 0; i < FS_DATA_BITMAP_LENGTH; i++) {
        if (1) {
	
        }
    }
    
    // 
    
    return ptr;
}

// read/write a block in the inode, starts from block 0 
bool inode_block_operation(Disk* disk, size_t inode_number, size_t block, uint8_t data[FS_BLOCK_SIZE], bool write) {
    log_debug("Tring to read inode block %lu", block);
    if (block > FS_MAX_FILE_SIZE/FS_BLOCK_SIZE) {
        log_error("[fs] Error, trying to get a block outside of the limits");
        return false;
    }
    
    Inode inode;
    size_t block_ptr = 0x0; //address of the block on the disk
    bool is_indirect_ptr = false;
    
    if (!get_inode(disk, inode_number, &inode)) {
        return false;
    }

    // find where the block is 
    if (block < FS_POINTERS_PER_INODE) { // it's a direct pointer
        block_ptr = inode.direct_pointers[block];
    } else { //it's an indirect pointer 
        is_indirect_ptr = true;
        block_ptr = inode.indirect_pointers[block];
        
    }
    
    // if pointer is set to 0 there is no block allocated
    if (block_ptr == 0x0 && !write) {
            log_error("[fs] Trying to read an unallocated block");
            return false;
        } else if (block_ptr == 0 && write) {
            log_debug("allocating a new direct block");
	    int block_ptr = allocate_ptr_block(disk);
	    if (block_ptr == -1) {
		log_error("cannot allocate new block");
		return false;
	    }
    }
    
    // if we're trying to read an block with indirect pointer
    if (is_indirect_ptr) {
	uint8_t ind_block_data[FS_BLOCK_SIZE] = {0};
        
        size_t ind_block = block / FS_POINTER_PER_BLOCK;
        size_t ind_block_offset = block - FS_POINTERS_PER_INODE - (ind_block * FS_POINTER_PER_BLOCK);
        log_debug("ind_block=%lu, ind_block_offset=%lu", ind_block, ind_block_offset);
        if (!disk_read_raw(disk, ind_block_data, FS_BLOCK_SIZE, inode.indirect_pointers[ind_block])) {
            log_error("[fs] Cannot read indirect pointer");
            return false;
        }

        //grab the pointer from the pointer block
        block_ptr = get_uint32(ind_block_data+(ind_block_offset * FS_POINTER_SIZE));
    }

    
    //check if ptr is an allocated block
    log_error("must implement allocating blocks");
    __asm__("jmp 0"); //CRASH
    

    //read/write the block
    log_debug("block ptr=%lu", block_ptr);
    if (write) {
        if (disk_write_raw(disk, data, FS_BLOCK_SIZE, block_ptr) != DISK_SUCCESS) {
            return false;
        }
    } else {
        if (disk_read_raw(disk, data, FS_BLOCK_SIZE, block_ptr) != DISK_SUCCESS) {
            return false;
        }
    }

    return true;
}

// if write is false it will read the inode and put it in the inode struct, if write is true it will
// write the inode the function was given to the disk
bool inode_operation(Disk* disk, size_t inode_number, Inode* inode, bool write) {
    log_debug("FS_MAX_INODES=%lu", FS_MAX_INODES);
    log_debug("FS_MAX_FILE_SIZE=%lu", FS_MAX_FILE_SIZE);
    
    int inode_addr;
    int ptr_offset = 0; 
    uint8_t inode_buf[FS_INODE_SIZE] = {0};
    log_debug("[fs] max_inodes: %u, start address=%u\n", FS_MAX_INODES, FS_INODE_TABLE_START_ADDR);

    //check if we are outside the inode table
    if ((inode_number+1) > FS_MAX_INODES) {
        log_error("[fs] trying to access an inode outside the inode table\n");
        return false;
    }
    inode_addr = FS_INODE_TABLE_START_ADDR + inode_number * FS_INODE_SIZE;
    log_debug("inode addr : %u\n", inode_addr);

    if (!write) {
	    //load the inode into the buffer	
        if(disk_read_raw(disk, inode_buf, FS_INODE_SIZE, inode_addr) != DISK_SUCCESS) {
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
    	write_uint64_arr(inode_buf+ptr_offset, inode->size);
    else
        inode->size = get_uint64(inode_buf+ptr_offset);
        
    ptr_offset += 8;

    if (write) 
        write_uint64_arr(inode_buf+ptr_offset, inode->atime);
    else
        inode->atime = get_uint64(inode_buf+ptr_offset);
        
    ptr_offset += 8;

    if (write) 
        write_uint64_arr(inode_buf+ptr_offset, inode->mtime);
    else
        inode->mtime = get_uint64(inode_buf+ptr_offset);
    
    ptr_offset += 8;
    for (int i = 0; i < FS_POINTERS_PER_INODE; i++) {
    	if (write) {
    		write_uint32_arr(inode_buf+ptr_offset, inode->direct_pointers[i]);	
    		log_debug("writing dp: %04X\n", inode->direct_pointers[i]);
    	} else
	        inode->direct_pointers[i] = get_uint32(inode_buf+ptr_offset);
        ptr_offset += 4;
    }
    for (int i = 0; i < FS_INDIRECT_POINTERS_PER_INODE; i++) {
    	if (write)
    		write_uint32_arr(inode_buf+ptr_offset, inode->indirect_pointers[i]);
    	else
	        inode->indirect_pointers[i] = get_uint32(inode_buf+ptr_offset);
        ptr_offset += 4;
    }
    
    if (write) {
	    //write inode buffer to disk 
	    if (disk_write_raw(disk, inode_buf, FS_INODE_SIZE, inode_addr) != DISK_SUCCESS) { 
	    	log_error("[fs] cannot write inode to disk\n");
	    	return false;
	    } 
	}
	    
    
    return true;
}

//return false if the inode is not properly set up
//also returns false if type is none
bool check_inode_integrity(Inode* inode) {
    if (inode->type == FS_TYPE_NONE || inode->type != FS_TYPE_FILE || inode->type != FS_TYPE_LINK || inode->type != FS_TYPE_DIR) {
        return false;
    }

    //check if every pointers point to a valid data block

    //check if size is matching the number of data block
    
    return true;
    
}



bool get_superblock(Disk *disk, SuperBlock *superblock) {
    if (!superblock_operation(disk, superblock, false)) {
        log_error("[fs] Error getting superblock\n");
        return false;
    }

    return true;
}

bool save_superblock(Disk *disk, SuperBlock *superblock) {
	if (!superblock_operation(disk, superblock, true)) {
        log_error("[fs] Error saving superblock\n");
        return false;
    }

    return true;
}

bool superblock_operation(Disk *disk, SuperBlock *superblock, bool write) {
    uint8_t superblock_buf[FS_BLOCK_SIZE] = {0};
    int ptr_offset = 0; 
    if (!write) {
		if (!read_block(disk, superblock_buf, 0)) {
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
		    write_uint32_arr(superblock_buf+ptr_offset, superblock->blocks_count);
    else
	    superblock->blocks_count = get_uint32(superblock_buf+ptr_offset);
    ptr_offset += 4;
    
    if (write)
	    write_uint32_arr(superblock_buf+ptr_offset, superblock->inode_blocks);
    else
	    superblock->inode_blocks = get_uint32(superblock_buf+ptr_offset);
    ptr_offset += 4;
    
    if (write)
	    write_uint32_arr(superblock_buf+ptr_offset, superblock->inodes_count);    
    else
		superblock->inodes_count = get_uint32(superblock_buf+ptr_offset);
    ptr_offset += 4;
    
    if (write)
	    write_uint32_arr(superblock_buf+ptr_offset, superblock->free_data_blocks_count);
    else
		superblock->free_data_blocks_count = get_uint32(superblock_buf+ptr_offset);

	if (write) {
		if (!write_block(disk, superblock_buf, 0)) {
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
bool read_block(Disk* disk, uint8_t data[FS_BLOCK_SIZE], size_t block) {
    int pos = block * FS_BLOCK_SIZE;
    
    if ((block * FS_BLOCK_SIZE) > disk_get_size(disk)) {
        log_error("[fs] trying to read fs block outside the disk\n");
        return false;
    }
    
    if (disk_read_raw(disk, data, FS_BLOCK_SIZE, pos) != DISK_SUCCESS) {
        log_error("[fs] cannot read the disk\n");
        return false;
    } 

    return true;
}

/* function similarly to read block */
bool write_block(Disk* disk, uint8_t data[FS_BLOCK_SIZE], size_t block) {
    int pos = block * FS_BLOCK_SIZE;
    
    if ((block * FS_BLOCK_SIZE) > disk_get_size(disk)) {
        log_error("[fs] trying to read fs block outside the disk\n");
        return false;
    }
    
    if (disk_write_raw(disk, data, FS_BLOCK_SIZE, pos) != DISK_SUCCESS) {
        log_error("[fs] cannot write the disk\n");
        return false;
    } 

    return true;
}


ssize_t allocate_free_block(FileSystem *fs)
{
    return 0;
}

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
    for (int i = 0; i < FS_POINTERS_PER_INODE; i++) {
        log_debug("pointer %d: %u\n", i, inode.direct_pointers[i]);
    }
    for (int i = 0; i < FS_INDIRECT_POINTERS_PER_INODE; i++) {
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
    get_inode(disk, 1, &inode2_read);

    uint8_t data[FS_BLOCK_SIZE] = {0};
    inode_block_operation(disk, 0, 0, data, 0);
    
    disk_close(disk);
}

