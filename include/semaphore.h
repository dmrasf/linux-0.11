#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <linux/sched.h>

typedef struct {
    char name[32];
    unsigned int value;
    struct task_struct *queue;
} sem_t;

extern sem_t * sem_open(const char *name, unsigned int value);
extern int sem_wait(sem_t *sem);
extern int sem_unlink(const char *name);
extern int sem_post(sem_t *sem);

#endif /* _SEMAPHORE_H_ */
