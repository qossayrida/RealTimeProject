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
#include "config.h"

Config config;
SharedMemory *shared_data;
int shm_id;


void handle_alarm();
void create_process(const char *program, int shm_id, int count);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (load_config(argv[1], &config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }

    shm_id = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
    shared_data = (SharedMemory *) shmat(shm_id, NULL, 0);
    memset(shared_data, 0, sizeof(SharedMemory));
    shared_data->config = config;

    create_process("./bin/counter_espionage_agency", shm_id, 1);
    create_process("./bin/resistance_group", shm_id, 1);


    signal(SIGALRM, handle_alarm);
    alarm(config.time_to_end);

    while (1) {
        sleep(1);
    }

    return 0;
}

void handle_alarm() {
    printf("Time is up!\n");
    exit(EXIT_SUCCESS);
}


void create_process(const char *program, int shm_id, int count) {
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            char shm_id_str[10];
            sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
            setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
            execlp(program, program, NULL);    // Replace process with new program
            perror("execlp failed");
            exit(1); // Exit if execlp fails
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
    }
}