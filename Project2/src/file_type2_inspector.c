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

void ensure_directory(const char *path);
void move(const char *source, const char *destination);
char* get_first_file();
void remove_spaces_and_newlines(char *str);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem= semget(1234, 1, 0666 );
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);


    printf("Type 2 Inspector: Started\n");
    ensure_directory(BACKUP_DIR); // Ensure UnProcessed directory exists

    while (shared_data->simulation_running) {

        sem_wait(sem); // Lock access to shared data
        char *first_element = get_first_file();
        char source[MAX_PATH_LENGTH];
        char destination[MAX_PATH_LENGTH];


        if (first_element != NULL) {
            snprintf(source, MAX_PATH_LENGTH, "%s%s",PROCESSED_DIR, first_element);
            snprintf(destination, MAX_PATH_LENGTH, "%s%s",BACKUP_DIR, first_element);
        } else {
            goto label;
        }

        struct stat file_stat;
        if (stat(source, &file_stat) == 0) {
            time_t current_time = time(NULL);
            double file_age = difftime(current_time, file_stat.st_mtime); // File age in days

            if (file_age > shared_data->config.processed_limit) {
                move(source, destination);

                shared_data->total_backup_files++;

            }
        }

        label:
        sem_post(sem); // Unlock access to shared data

    }


    return 0;
}

void ensure_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        mkdir(path, 0700); // Create directory with read/write/execute permissions
        printf("Created directory: %s\n", path);
    }
}

void move(const char *source, const char *destination) {
    char command[512];
    snprintf(command, sizeof(command), "mv %s %s",source, destination);
    printf("Executing: %s \n", command);
    system(command);
}

char* get_first_file() {
    char command[512];
    snprintf(command, sizeof(command), "ls %s -v | head -n 1", PROCESSED_DIR);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return NULL;
    }

    static char result[512];
    if (fgets(result, sizeof(result), fp) != NULL) {
        pclose(fp);
        remove_spaces_and_newlines(result);
        return result;
    }

    pclose(fp);
    return NULL;
}


void remove_spaces_and_newlines(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != ' ' && *src != '\n') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}