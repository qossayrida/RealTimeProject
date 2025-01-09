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


void* counter_espionage_agency_member(void* arg);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    config = shared_data->config;

    pthread_t threads[config.number_of_member_in_counter_espionage_agency];
    int args[config.number_of_member_in_counter_espionage_agency];

    for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {
        args[i] = i;
        if (pthread_create(&threads[i], NULL, counter_espionage_agency_member, &args[i]) != 0) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {
        pthread_join(threads[i], NULL);
    }


    return 0;
}


void* counter_espionage_agency_member(void* arg) {
    int id = *(int*)arg;
    printf("Counter espionage agency member %d started\n", id);
    printf("Counter espionage agency member %d finished\n", id);
    return NULL;
}