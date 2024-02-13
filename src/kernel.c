#include <kernel.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int sys_open(const char *pathname, bool should_create)
{
 int flags;

 if (should_create)
 {
  flags = O_CREAT | O_RDWR;
 }
 else
 {
  flags = O_RDWR;
 }

 return open(pathname, flags, 0660);
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

 int linux_whence;

 switch (whence)
 {
 case W_SET:
  linux_whence = SEEK_SET;
  break;
 case W_END:
  linux_whence = SEEK_END;
  break;
 default:
  return -1; /* Invalid "whence" argument */
  break;
 }

 return lseek(fd, offset, linux_whence);
}

int sys_stat(const char *restrict pathname, struct stat *statbuf)
{
 return -1; // stat(pathname, statbuf);
}

int sys_fstat(int fd, struct stat *statbuf)
{
 return -1; // fstat(fd, statbuf);
}

int sys_mkdir(const char *pathname, mode_t mode)
{
 return mkdir(pathname, mode);
}

int sys_rmdir(const char *pathname)
{
 return rmdir(pathname);
}

bool sys_truncate(const char *pathname, size_t new_size)
{
 return truncate(pathname, new_size);
}

size_t sys_rename(char *oldpath, char *newpath)
{
 return rename(oldpath, newpath);
}

char **sys_readdir(const char *pathname)
{
 return NULL;
}

/* internal data structures */

/* internal functions */
