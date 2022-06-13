/*
 *	linux/kernel/sys.c
 *
 *	(C) 1991  Linus Torvalds
 */

#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <semaphore.h>
#include <asm/system.h>
#include <string.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/shm.h>
#include <linux/mm.h>

int sys_ftime()
{
    return -ENOSYS;
}

int sys_break()
{
    return -ENOSYS;
}

int sys_ptrace()
{
    return -ENOSYS;
}

int sys_stty()
{
    return -ENOSYS;
}

int sys_gtty()
{
    return -ENOSYS;
}

int sys_rename()
{
    return -ENOSYS;
}

int sys_prof()
{
    return -ENOSYS;
}

int sys_setregid(int rgid, int egid)
{
    if (rgid > 0) {
        if ((current->gid == rgid) || suser())
            current->gid = rgid;
        else
            return (-EPERM);
    }
    if (egid > 0) {
        if ((current->gid == egid) || (current->egid == egid) || (current->sgid == egid) || suser())
            current->egid = egid;
        else
            return (-EPERM);
    }
    return 0;
}

int sys_setgid(int gid)
{
    return (sys_setregid(gid, gid));
}

int sys_acct()
{
    return -ENOSYS;
}

int sys_phys()
{
    return -ENOSYS;
}

int sys_lock()
{
    return -ENOSYS;
}

int sys_mpx()
{
    return -ENOSYS;
}

int sys_ulimit()
{
    return -ENOSYS;
}

int sys_time(long *tloc)
{
    int i;

    i = CURRENT_TIME;
    if (tloc) {
        verify_area(tloc, 4);
        put_fs_long(i, (unsigned long *)tloc);
    }
    return i;
}

/*
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_setreuid(int ruid, int euid)
{
    int old_ruid = current->uid;

    if (ruid > 0) {
        if ((current->euid == ruid) || (old_ruid == ruid) || suser())
            current->uid = ruid;
        else
            return (-EPERM);
    }
    if (euid > 0) {
        if ((old_ruid == euid) || (current->euid == euid) || suser())
            current->euid = euid;
        else {
            current->uid = old_ruid;
            return (-EPERM);
        }
    }
    return 0;
}

int sys_setuid(int uid)
{
    return (sys_setreuid(uid, uid));
}

int sys_stime(long *tptr)
{
    if (!suser())
        return -EPERM;
    startup_time = get_fs_long((unsigned long *)tptr) - jiffies / HZ;
    return 0;
}

int sys_times(struct tms *tbuf)
{
    if (tbuf) {
        verify_area(tbuf, sizeof *tbuf);
        put_fs_long(current->utime, (unsigned long *)&tbuf->tms_utime);
        put_fs_long(current->stime, (unsigned long *)&tbuf->tms_stime);
        put_fs_long(current->cutime, (unsigned long *)&tbuf->tms_cutime);
        put_fs_long(current->cstime, (unsigned long *)&tbuf->tms_cstime);
    }
    return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
    if (end_data_seg >= current->end_code && end_data_seg < current->start_stack - 16384)
        current->brk = end_data_seg;
    return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
    int i;

    if (!pid)
        pid = current->pid;
    if (!pgid)
        pgid = current->pid;
    for (i = 0; i < NR_TASKS; i++)
        if (task[i] && task[i]->pid == pid) {
            if (task[i]->leader)
                return -EPERM;
            if (task[i]->session != current->session)
                return -EPERM;
            task[i]->pgrp = pgid;
            return 0;
        }
    return -ESRCH;
}

int sys_getpgrp(void)
{
    return current->pgrp;
}

int sys_setsid(void)
{
    if (current->leader && !suser())
        return -EPERM;
    current->leader = 1;
    current->session = current->pgrp = current->pid;
    current->tty = -1;
    return current->pgrp;
}

int sys_uname(struct utsname *name)
{
    static struct utsname thisname = {
        "linux .0", "nodename", "release ", "version ", "machine "};
    int i;

    if (!name) return -ERROR;
    verify_area(name, sizeof *name);
    for (i = 0; i < sizeof *name; i++)
        put_fs_byte(((char *)&thisname)[i], i + (char *)name);
    return 0;
}

int sys_umask(int mask)
{
    int old = current->umask;

    current->umask = mask & 0777;
    return (old);
}

#define MAX_SEM_NUM 10

static sem_t sem_table[MAX_SEM_NUM];
static int sem_table_nr = -1;

sem_t *sys_sem_open(const char *name, unsigned int value)
{
    int i = 0;
    char str[100];
    char c;

    for (i = 0; i < 100; i++) {
        c = get_fs_byte(name + i);
        str[i] = c;
        if (c == '\0')
            break;
    }

    for (i = 0; i <= sem_table_nr; i++)
        if (strcmp(sem_table[i].name, str) == 0)
            break;

    if (i >= MAX_SEM_NUM)
        printk("no other space for new sem\n");
    else if (i > sem_table_nr) {
        sem_table_nr = i;

        i = 0;
        do {
            sem_table[sem_table_nr].name[i] = str[i];
        } while (str[i++] != '\0');
        sem_table[sem_table_nr].value = value;
        sem_table[sem_table_nr].queue = NULL;

        return &(sem_table[sem_table_nr]);
    } else {
        printk("already have same sem\n");
        return &(sem_table[i]);
    }
    return NULL;
}

int sys_sem_unlink(const char *name)
{
    int i = 0;
    char str[100];
    char c;

    for (i = 0; i < 100; i++) {
        c = get_fs_byte(name + i);
        str[i] = c;
        if (c == '\0')
            break;
    }

    for (i = 0; i <= sem_table_nr; i++)
        if (strcmp(sem_table[i].name, str) == 0)
            break;

    if (i > sem_table_nr)
        printk("not find sem in sem_table\n");
    else {
        int j = 0;
        for (j = i; j < sem_table_nr; j++)
            sem_table[j] = sem_table[j + 1];
        sem_table_nr--;
        printk("sem: %s unlink\n", name);
    }
    return 0;
}

int sys_sem_wait(sem_t *sem)
{
    cli();
    // 没有信号量，将进程添加到等待队列中
    while (sem->value <= 0)
        sleep_on(&(sem->queue));
    sem->value--;
    sti();
    return 0;
}

int sys_sem_post(sem_t *sem)
{
    sem->value++;
    // 只要有值，去唤醒等待队列中进程
    if (sem->value > 0)
        wake_up(&(sem->queue));
    return 0;
}

int sys_shmget(int key, size_t size, int shmflg)
{
    int i = 0;
    unsigned long p;

    for (i = 0; i < SHM_SIZE; i++)
        if (shm_node[i].key == key)
            return i;

    if (size > PAGE_SIZE)
        return -EFAULT;

    p = get_free_page();
    if (p == 0)
        return -ENOMEM;

    printk("get_free_page %d\n", p);

    for (i = 0; i < SHM_SIZE; i++)
        if (shm_node[i].key == 0) {
            shm_node[i].key = key;
            shm_node[i].size = size;
            shm_node[i].page = p;
            return i;
        }

    return -ENOMEM;
}

void *sys_shmat(int shmid, const void *shmaddr, int shmflg)
{
    if (shmid < 0 || shmid > SHM_SIZE || shm_node[shmid].key == 0)
        return NULL;
    // current->brk + current->start_code 相当于使用段机制得出的线性地址
    put_page(shm_node[shmid].page, current->brk + current->start_code);
    current->brk += PAGE_SIZE;
    printk("brk =  %d, start_code = %d\n", current->brk, current->start_code);
    // 段偏移地址
    return (void *)(current->brk - PAGE_SIZE);
}
