/*
 * iam()  whoami() 系统调用实验
 */

#include <asm/segment.h>
#include <errno.h>
#include <linux/kernel.h>

#define BUFSIZE 24

static char NAME[BUFSIZE];

int sys_iam(const char* name)
{
    int i;
    char c;

    for (i = 0; i < BUFSIZE; i++) {
        c = get_fs_byte(name + i);
        NAME[i] = c;
        if (c == '\0') {
            break;
        }
    }

    if (i == BUFSIZE) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int sys_whoami(char* name, unsigned int size)
{
    int i;

    for (i = 0; i < BUFSIZE; i++) {
        put_fs_byte(NAME[i], name + i);
        if (NAME[i] == '\0' || i + 1 == size)
            break;
    }

    if (i > size) {
        errno = EINVAL;
        return -1;
    }

    return i;
}
