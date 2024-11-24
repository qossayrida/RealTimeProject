
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <string.h>
#include "config.h"


int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    sem_t *sem = sem_open("/sim_sem", 0); // Open semaphore
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);

    while (shared_data->simulation_running) {
        sem_wait(sem);
        shared_data->total_generated_files++;
        printf("File generator: Generated file %d\n", shared_data->total_generated_files);
        sem_post(sem);
        sleep(rand() % 3 + 1); // Simulate random delay
    }

    shmdt(shared_data);
    return 0;
}
