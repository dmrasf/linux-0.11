#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdio.h>
#include "shm_key.h"
#include <stddef.h>

int main(int argc, char *argv[])
{
    int i;
    int shm_id;
    sem_t *full, *mutex, *empty;
    int *p;

    full = sem_open("full", 0);
    mutex = sem_open("mutex", 1);
    empty = sem_open("empty", 10);
    shm_id = shmget(SHM_KEY, 0, 0);
    p = (int *)shmat(shm_id, NULL, 0);

    for (i = 0; i < M; i++) {
        sem_wait(full);
        sem_wait(mutex);
        printf("%d\n", *(p + i % 10));
        sem_post(mutex);
        sem_post(empty);
    }

    sem_unlink("full");
    sem_unlink("mutex");
    sem_unlink("empty");
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}
