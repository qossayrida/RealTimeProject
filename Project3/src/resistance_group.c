#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "config.h"


Config config;

void* resistant_group_member(void* arg);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    config = shared_data->config;

    int number_of_member_in_resistant_group = rand() % (config.max_number_member_in_resistant_group - config.min_number_member_in_resistant_group + 1) + config.min_number_member_in_resistant_group;
    pthread_t threads[number_of_member_in_resistant_group];
    int args[number_of_member_in_resistant_group];

    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        args[i] = i;
        if (pthread_create(&threads[i], NULL, resistant_group_member, &args[i]) != 0) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        pthread_join(threads[i], NULL);
    }


    return 0;
}


void* resistant_group_member(void* arg) {
    int id = *(int*)arg;
    printf("Resistant group member %d started\n", id);
    printf("Resistant group member %d finished\n", id);
    return NULL;
}

