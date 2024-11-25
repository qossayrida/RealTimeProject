#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include "config.h"

void ensure_directory(const char *path);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);


    key_t key = ftok("queuefile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    ensure_directory(PROCESSED_DIR);

    Message msg;
    while (shared_data->simulation_running) {
        // Receive message
        if (msgrcv(msgid, &msg, sizeof(msg), 1, IPC_NOWAIT) > 0) {
            printf("Mover: Received %s.csv\n", msg.file_number);

            char destination[30];
            snprintf(destination, 30, "%s%s.csv", PROCESSED_DIR, msg.file_number);

            char source[30];
            snprintf(source, 30, "%s%s.csv", HOME_DIR, msg.file_number);

            // Move file
            char command[512];
            snprintf(command, sizeof(command), "mv %s %s",source, destination);
            printf("Executing: %s\n", command);
            system(command);  // Use `rename()` for safety in production
        }
    }

    shmdt(shared_data);
    return 0;
}


// Function to ensure HOME_DIR exists
void ensure_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        mkdir(path, 0700); // Create directory with read/write/execute permissions
        printf("Created directory: %s\n", path);
    }
}