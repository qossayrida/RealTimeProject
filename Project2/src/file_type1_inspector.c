#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/shm.h>
#include "config.h"
#include "sem.h"

int fifo_fd;

void ensure_directory(const char *path);

void move_to_unprocessed(const char *filename);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem = semget(1234, 1, 0666);
    SharedData *shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    fifo_fd = open(FIFO_NAME, O_WRONLY);

    printf("Type 1 Inspector: Started\n");
    ensure_directory(UNPROCESSED_DIR); // Ensure UnProcessed directory exists


    while (shared_data->simulation_running) {
        sem_wait(sem); // Lock access to shared data

        char file_number[MAX_PATH_LENGTH];
        snprintf(file_number, MAX_PATH_LENGTH, "%d",
                 shared_data->total_calculated_files + shared_data->total_unprocessed_files);

        char source[MAX_PATH_LENGTH * 2];
        snprintf(source, MAX_PATH_LENGTH * 2, "%s%s.csv", HOME_DIR, file_number);


        ////////////////////////////////////////////////////////////////////////////////
        // File inspection logic goes here (Make sure if find one check the next one)
        ////////////////////////////////////////////////////////////////////////////////
        struct stat file_stat;
        if (stat(source, &file_stat) == 0) {
            time_t current_time = time(NULL);
            double file_age = difftime(current_time, file_stat.st_mtime); // File age in days

            if (file_age > shared_data->config.file_age_limit) {
                move_to_unprocessed(file_number);
                send_info(fifo_fd, TYPE1_INSPECTOR, getpid(), file_number);
                shared_data->total_unprocessed_files++;
            }
        }

        sem_post(sem); // Unlock access to shared data
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
    char destination[MAX_PATH_LENGTH];
    snprintf(destination, MAX_PATH_LENGTH, "%s%s.csv", UNPROCESSED_DIR, filename);

    char source[MAX_PATH_LENGTH];
    snprintf(source, MAX_PATH_LENGTH, "%s%s.csv", HOME_DIR, filename);

    // Move file
    char command[512];
    snprintf(command, sizeof(command), "mv %s %s", source, destination);
    system(command);
}
