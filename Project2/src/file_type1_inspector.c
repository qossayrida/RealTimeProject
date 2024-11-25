#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>
#include <sys/shm.h>
#include "config.h"

void ensure_directory(const char *path);
void move_to_unprocessed(const char *filename);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
//  sem_t *sem = sem_open("/sim_sem", 0); // Open semaphore
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);


    printf("Type 1 Inspector: Started\n");
    ensure_directory(UNPROCESSED_DIR); // Ensure UnProcessed directory exists


    while (shared_data->simulation_running) {


    }


    shmdt(shared_data);
    return 0;
}


void ensure_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        mkdir(path, 0700); // Create directory with read/write/execute permissions
        printf("Created directory: %s\n", path);
    }
}


void move_to_unprocessed(const char *filename) {
    char destination[600];
    snprintf(destination, 600, "%s%s", UNPROCESSED_DIR, filename);

    if (rename(filename, destination) == 0) {
        printf("Moved file: %s to UnProcessed directory\n", filename);
    } else {
        perror("Failed to move file");
    }
}
