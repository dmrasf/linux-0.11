#ifndef _SHM_H_
#define _SHM_H_

#include <stddef.h>

#define SHM_SIZE 20

struct shm_node_t {
    int key;
    size_t size;
    unsigned long page;
} shm_node[SHM_SIZE];

extern int shmget(int key, size_t size, int shmflg);
extern void *shmat(int shmid, const void *shmaddr, int shmflg);

#endif /* _SHM_H_ */
