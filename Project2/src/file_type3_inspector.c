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

void remove_file(const char *filename);

char *get_first_file();

void remove_spaces_and_newlines(char *str);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem = semget(1234, 1, 0666);
    SharedData *shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    fifo_fd = open(FIFO_NAME, O_WRONLY);

    printf("Type 3 Inspector: Started\n");

    while (shared_data->simulation_running) {
        sem_wait(sem); // Lock access to shared data


        char *first_element = get_first_file();
        char source[MAX_PATH_LENGTH];

        if (first_element != NULL) {
            snprintf(source, MAX_PATH_LENGTH, "%s%s", BACKUP_DIR, first_element);
        } else {
            goto label;
        }

        struct stat file_stat;
        if (stat(source, &file_stat) == 0) {
            time_t current_time = time(NULL);
            double file_age = difftime(current_time, file_stat.st_ctime);
            if (file_age > shared_data->config.backup_limit) {
                remove_file(source);
                send_info(fifo_fd, TYPE3_INSPECTOR, getpid(), first_element);
                shared_data->total_deleted_files++;
            }
        }

        label:
        sem_post(sem); // Unlock access to shared data
    }
    return 0;
}


void remove_file(const char *source) {
    char command[512];
    snprintf(command, sizeof(command), "rm %s ", source);
    system(command);
}


char *get_first_file() {
    char command[512];
    snprintf(command, sizeof(command), "ls %s -v | head -n 1", BACKUP_DIR);

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