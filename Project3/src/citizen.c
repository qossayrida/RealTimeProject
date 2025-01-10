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

int my_type;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <type>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "0") == 0) {
        my_type = SPY;
    } else if (strcmp(argv[1], "1") == 0) {
        my_type = NORMAL;
    } else {
        fprintf(stderr, "Invalid type\n");
        return EXIT_FAILURE;
    }

    printf("Citizen: %s\n", my_type == SPY ? "Spy" : "Normal");


    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    Config config = shared_data->config;

    return 0;
}


