#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_M 500
#define MAX_BUFFER 10
#define PRO_NUM 3

int main(int argc, char* argv[])
{
    char str[5];
    int fd;
    int readstrlen, first;
    sem_t *mutex, *empty, *full;
    char c;

    sem_unlink("mutex");
    mutex = sem_open("mutex", O_CREAT, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("create sem_mutex error");
        exit(-1);
    }
    sem_unlink("empty");
    empty = sem_open("empty", O_CREAT, 0644, MAX_BUFFER);
    if (empty == SEM_FAILED) {
        perror("create sem_empty error");
        exit(-1);
    }
    sem_unlink("full");
    full = sem_open("full", O_CREAT, 0644, 0);
    if (full == SEM_FAILED) {
        perror("create sem_full error");
        exit(-1);
    }

    fd = open("buffer", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open file buffer error");
        exit(-1);
    }

    int pid;
    for (int i = 0; i < PRO_NUM; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork error");
            exit(-1);
        }
        if (pid == 0) {
            pid = getpid();
            while (1) {
                sem_wait(full);
                sem_wait(mutex);
                lseek(fd, 0, SEEK_SET);
                readstrlen = 0;
                while (1) {
                    if (read(fd, &c, 1) == -1)
                        break;
                    str[readstrlen] = c;
                    readstrlen++;
                    if (c == '\n')
                        break;
                }
                str[readstrlen] = '\0';
                printf("consumer%d => %d:\t%s", i, pid, str);
                first = 0;
                while (1) {
                    lseek(fd, first + readstrlen, SEEK_SET);
                    if (read(fd, &c, 1) != 1)
                        break;
                    lseek(fd, first, SEEK_SET);
                    write(fd, &c, 1);
                    first++;
                }
                ftruncate(fd, first);
                sem_post(mutex);
                sem_post(empty);
            }
            exit(0);
        }
    }

    for (int i = 0; i < MAX_M; i++) {
        sem_wait(empty);
        sem_wait(mutex);
        sprintf(str, "%d\n", i);
        lseek(fd, 0, SEEK_END);
        write(fd, str, strlen(str));
        printf("producer => %d\n", i);
        sem_post(mutex);
        sem_post(full);
    }

    sem_close(full);
    sem_close(empty);
    sem_close(mutex);
    close(fd);

    while (wait(NULL) > 0)
        ;
    return 0;
}
