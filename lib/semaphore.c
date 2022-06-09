// 这些东西应该写到测试文件里，否则找不到函数。我不知道为什么，我认为它已经编译到内核去了，但函数调用找不到

#define __LIBRARY__

#include <unistd.h>
#include <semaphore.h>

_syscall2(sem_t *, sem_open, const char *, name, unsigned int, value)
_syscall1(int, sem_wait, sem_t *, sem)
_syscall1(int, sem_unlink, const char *, name)
_syscall1(int, sem_post, sem_t *, sem)
