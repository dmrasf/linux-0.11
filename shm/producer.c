#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "shm_key.h"
#include <stddef.h>

int main(int argc, char *argv[])
{
    int i;
    int shm_id;
    sem_t *full, *mutex, *empty;
    int *p;

    full = sem_open("full", O_CREAT | O_EXCL, 0666, 0);
    mutex = sem_open("mutex", O_CREAT | O_EXCL, 0666, 1);
    empty = sem_open("empty", O_CREAT | O_EXCL, 0666, 10);
    shm_id = shmget(SHM_KEY, 10, IPC_CREAT | IPC_EXCL | 0666);
    p = (int *)shmat(shm_id, NULL, 0);

    for (i = 0; i < M; i++) {
        sem_wait(empty);
        sem_wait(mutex);
        *(p + i % 10) = i;
        sem_post(mutex);
        sem_post(full);
    }
    return 0;
}
