#include <kernel.h>

#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>

int sys_open(const char *pathname, int flags)
{
	return open(pathname, flags);
}

int sys_close(int fd)
{
	return close(fd);
}

ssize_t sys_read(int fd, char *buf, size_t count)
{
	return read(fd, buf, count);
}

ssize_t sys_write(int fd, const char *buf, size_t count)
{
	return write(fd, buf, count);
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
	return lseek(fd, offset, whence);
}

int sys_stat(const char *restrict pathname, struct stat *statbuf) {
	return stat(pathname, statbuf);
}

int sys_fstat(int fd, struct stat *statbuf) {
	return fstat(fd, statbuf);
}

/* internal data structures */

/* internal functions */


