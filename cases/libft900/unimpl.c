#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int _close(int fd)
{
  return 0;
}

int _fstat(int fd, struct stat *buf)
{
  return -1;
}

int _isatty()
{
  return 0;
}

int _lseek()
{
  return 0;
}

int _unlink()
{
  return 0;
}

int _open()
{
  return 0;
}

int _gettimeofday() { return 0; }
int _kill() { return 0; }
int _getpid() { return 0; }
int _link() { return 0; }
