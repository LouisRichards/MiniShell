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

int sys_stat(const char *restrict pathname, struct sys_stat *statbuf)
{
 int res;
 struct stat posix_statbuf;
 res = stat(pathname, &posix_statbuf);
 if (res == -1)
 {
  // logger erreur
  printf("Erreur lectur fichier\n"); // utiliser log_error
  return -1;
 }

 statbuf->sys_ino = posix_statbuf.st_ino;
 statbuf->sys_atime = posix_statbuf.st_atime;
 statbuf->sys_blocks = posix_statbuf.st_blocks;
 statbuf->sys_size = posix_statbuf.st_size;
 statbuf->sys_type = 0;
 if (S_ISREG(posix_statbuf.st_mode))
 {
  statbuf->sys_type = SYS_FILE;
 }
 else if (S_ISDIR(posix_statbuf.st_mode))
 {
  statbuf->sys_type = SYS_DIR;
 }
 else if (S_ISLNK(posix_statbuf.st_mode))
 {
  statbuf->sys_type = SYS_LINK;
 }
 else
 {
  printf("FILE NOT SUPPORTED BY FS FOR THE MOMENT\n"); // TODO use log_error
  exit(1);
 }

 return 0;
}

int sys_fstat(int fd, struct sys_stat *statbuf)
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

bool sys_readdir(const char *pathname, struct sys_dirent *dirent)
{
 // TODO replace this with sgf constants
 const size_t MAX_DIR_SIZE = 512;
 const size_t MAX_PATH_LENGTH = 512;

 DIR *dir = opendir(pathname);
 struct dirent *posix_dirent;

 dirent->names = malloc(sizeof(char *) * MAX_DIR_SIZE);
 dirent->paths = malloc(sizeof(char *) * MAX_DIR_SIZE);

 int i = 0;
 while ((posix_dirent = readdir(dir)) != NULL)
 {
  char *name = posix_dirent->d_name;
  char abs_path[MAX_PATH_LENGTH];
  realpath(name, abs_path);

  dirent->names[i] = malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(dirent->names[i], name);
  dirent->paths[i] = malloc(sizeof(char) * (strlen(abs_path) + 1));
  strcpy(dirent->paths[i], abs_path);
  // printf("folder : %s, abs_path: %s\n",name, abs_path);
  i++;
 }
 dirent->entries = i;

 closedir(dir);
 return true;
}

void sys_free_dirent(struct sys_dirent *dirent)
{
 for (int i = 0; i < dirent->entries; i++)
 {
  free(dirent->names[i]);
  free(dirent->paths[i]);
 }
 free(dirent->names);
 free(dirent->paths);
}