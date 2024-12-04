#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "config.h"
#include "sem.h"

int fifo_fd;

void ensure_directory(const char *path);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedData *shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    fifo_fd = open(FIFO_NAME, O_WRONLY);


    key_t key = ftok("queuefile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    ensure_directory(PROCESSED_DIR);

    Message msg;
    while (shared_data->simulation_running) {
        // Receive message
        if (msgrcv(msgid, &msg, sizeof(msg), 1, IPC_NOWAIT) > 0) {
            send_info(fifo_fd, FILE_MOVER, getpid(), msg.file_number);

            char destination[MAX_PATH_LENGTH];
            snprintf(destination, MAX_PATH_LENGTH, "%s%s.csv", PROCESSED_DIR, msg.file_number);

            char source[MAX_PATH_LENGTH];
            snprintf(source, MAX_PATH_LENGTH, "%s%s.csv", HOME_DIR, msg.file_number);

            // Move file
            char command[512];
            snprintf(command, sizeof(command), "mv %s %s", source, destination);
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