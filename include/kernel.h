/* Simple Kernel header that offers open, close, read, write, lseek, and ioctl
 * system calls.  The kernel is implemented as a single process that
 * runs in user mode.
 */
#include <sys/types.h>
#include <stdio.h>


#ifndef KERNEL_H
#define KERNEL_H

/* Kernel data structures */
struct stat {
	
};

/* Kernel system call prototypes */
int sys_open(const char *pathname, int flags);

int sys_close(int fd);

ssize_t sys_read(int fd, char *buf, size_t count);

ssize_t sys_write(int fd, const char *buf, size_t count);

off_t sys_lseek(int fd, off_t offset, int whence);

int sys_stat(const char *restrict pathname, struct stat *statbuf);

int sys_fstat(int fd, struct stat *statbuf);


#endif /* KERNEL_H */
