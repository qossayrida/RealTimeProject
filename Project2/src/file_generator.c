#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "sem.h"

int fifo_fd;
void ensure_directory(const char *path);
float generate_random_value(int miss_percentage);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem= semget(1234, 1, 0666 );
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    fifo_fd = open(FIFO_NAME, O_WRONLY);

    srand(time(NULL) ^ getpid()); // Seed randomness with time and PID
    ensure_directory(HOME_DIR); // Ensure HOME_DIR exists


    while (shared_data->simulation_running) {
        sem_wait(sem); // Lock access to shared data
        int file_number = shared_data->total_generated_files++;
        sem_post(sem); // Unlock access to shared data

        // Create file name
        char filename[30];
        snprintf(filename, 30, "%s%d.csv", HOME_DIR, file_number);

        // Open file for writing
        FILE *file = fopen(filename, "w");
        if (!file) {
            perror("Failed to create file");
            continue;
        }

        // Get random rows and columns within config range
        int rows = rand() % (shared_data->config.default_upper_rows - shared_data->config.default_lower_rows + 1) + shared_data->config.default_lower_rows;
        int columns = rand() % (shared_data->config.default_upper_columns - shared_data->config.default_lower_columns + 1) + shared_data->config.default_lower_columns;

        // Generate and write data to the file
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                float value = generate_random_value(shared_data->config.miss_percentage);
                if (value == -1) {
                    fprintf(file, "-"); // Indicate missing values
                } else {
                    fprintf(file, "%.2f", value); // Write random decimal
                }

                if (j < columns - 1)
                    fprintf(file, ","); // Add comma between values
            }
            fprintf(file, "\n"); // Newline at the end of each row
        }

        fclose(file); // Close the file
        send_info(fifo_fd, FILE_GENERATOR, getpid(), filename);

        sleep((rand() % (shared_data->config.max_generate_time - shared_data->config.min_generate_time + 1)) + shared_data->config.min_generate_time); // Simulate random delay
    }

    // Cleanup
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

float generate_random_value(int miss_percentage) {
    int random_value = rand() % 100; // Generate a random value between 0-99
    if (random_value < miss_percentage) {
        return -1; // Mark as missing (-1 indicates missing value)
    }
    return ((float)rand() / RAND_MAX) * 100; // Random decimal between 0 and 100
}
