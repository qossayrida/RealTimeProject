#ifndef INCLUDE_SEM_H
#define INCLUDE_SEM_H


#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


int init_semaphore() {
    int semid = semget(1234, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }

    // Initialize semaphore value to 1 (binary semaphore)
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl failed");
        exit(1);
    }
    return semid;
}

void sem_wait(int semid) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;  // Decrement semaphore
    sem_op.sem_flg = 0;

    if (semop(semid, &sem_op, 1) == -1) {
        perror("semop wait failed");
        exit(1);
    }
}

// V (signal) operation
void sem_post(int semid) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;  // Increment semaphore
    sem_op.sem_flg = 0;

    if (semop(semid, &sem_op, 1) == -1) {
        perror("semop signal failed");
        exit(1);
    }
}

#endif //INCLUDE_SEM_H
