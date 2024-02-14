/* Simple Kernel header that offers open, close, read, write, lseek, and ioctl
 * system calls.  The kernel is implemented as a single process that
 * runs in user mode.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#ifndef KERNEL_H
#define KERNEL_H

typedef int inode_t; /* file descriptor type (int) */

enum whence
{
 W_SET = 0,
 W_END = 1
};

/* Kernel data structures */
struct sys_stat
{
 inode_t sys_ino;  /*inode number*/
 size_t sys_atime; /* time of last access */
 size_t sys_mtime; /* time of last data modification */
 int sys_blocks;   /* blocks allocated for file */
 size_t sys_size;  /* file size */
 int sys_type;     /* file type */
};

struct sys_dirent
{
 int entries;  // number of entries
 char **names; // names of the folders
 char **paths; // absolute path
};

enum filetype
{
 SYS_FILE,
 SYS_DIR,
 SYS_LINK
};

/* Kernel system call prototypes */

/// @brief
/// @param pathname
/// @param should_create
/// @return file descriptor or -1 if error
int sys_open(const char *pathname, bool should_create); // High prio !!

int sys_close(int fd); // Med prio !

ssize_t sys_read(int fd, char *buf, size_t count); // High prio !!

ssize_t sys_write(int fd, const char *buf, size_t count); // High prio !!

off_t sys_lseek(int fd, off_t offset, int whence); // TODO: Whence -> SET END simple_lseek(fd, offset, whence)

int sys_stat(const char *restrict pathname, struct sys_stat *statbuf); // Med prio

int sys_fstat(int fd, struct sys_stat *statbuf); // Low prio

int sys_mkdir(const char *pathname, mode_t mode); // High prio !!

int sys_rmdir(const char *pathname); // High prio !!

bool sys_truncate(const char *pathname, size_t new_size); // Low prio

size_t sys_rename(char *oldpath, char *newpath); // High prio !!

bool sys_readdir(const char *pathname, struct sys_dirent *dirent); // High prio !!

#endif /* KERNEL_H */
