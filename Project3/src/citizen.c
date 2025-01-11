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

    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    Config config = shared_data->config;

    printf("Citizen: %s\n", my_type == SPY ? "Spy" : "Normal");

    while (shared_data->simulation_status) {
        if (shared_data->array_counter != 0) {

            int resistance_group_to_meet = rand() % shared_data->array_counter;

            if (shared_data->communication_pid[resistance_group_to_meet][2] == 2){
                int resistance_group_pid = shared_data->communication_pid[resistance_group_to_meet][0];

                char fifo_name[256];
                snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_for_resistance_group_%d", resistance_group_pid);
                int fifo_fd = open(fifo_name, O_WRONLY);
                Information info;
                info.id = getpid();
                int time_spent = rand() % 4 + 1;
                snprintf(info.content, sizeof(info.content), "%d",time_spent);
                write(fifo_fd, &info, sizeof(Information));
                kill(resistance_group_pid, SIGUSR1);
                close(fifo_fd);

                sleep(time_spent);

                if (my_type == SPY) {
                    info.type = CITIZEN;
                    int enemy_pid = shared_data->communication_pid[resistance_group_to_meet][1];
                    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_to_write_at_enemy_%d", enemy_pid);
                    int enemy_fifo_fd = open(fifo_name, O_WRONLY);
                    write(enemy_fifo_fd, &info, sizeof(Information));
                    close(enemy_fifo_fd);
                }

                sleep(rand() % 3 + 1);
            }
        }
    }




    return 0;
}


