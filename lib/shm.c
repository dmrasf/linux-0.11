#define __LIBRARY__

#include <stddef.h>
#include <unistd.h>
#include <sys/shm.h>

_syscall3(int, shmget, int, key, size_t, size, int, shmflg)
_syscall3(void *, shmat, int, shmid, const void *, shmaddr, int, shmflg)
