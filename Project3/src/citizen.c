#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include "config.h"
#include "sem.h"

int my_type;
int fifo_gui;

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <type> <pipe>\n", argv[0]);
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
    SharedMemory *shared_data = (SharedMemory *) shmat(shm_id, NULL, 0);

    char *pipe_fd_str = argv[2];
    fifo_gui = atoi(pipe_fd_str);
    if (fifo_gui <= 0) {
        fprintf(stderr, "Invalid pipe file descriptor\n");
        exit(EXIT_FAILURE);
    }


    send_info_to_gui(fifo_gui, CITIZEN_CREATED, getpid(), my_type == SPY ? "Spy" : "Normal");

    srand(time(NULL));
    int sem = semget(1234, 1, 0666);


    while (shared_data->simulation_status) {
        if (shared_data->array_counter != 0) {

            sem_wait(sem);
            int resistance_group_to_meet = rand() % shared_data->array_counter;
            int access = shared_data->communication_pid[resistance_group_to_meet][2] == 2 ? 1 : 0;
            if (access) {
                shared_data->communication_pid[resistance_group_to_meet][2]++;
            }
            sem_post(sem);

            if (access) {
                int resistance_group_pid = shared_data->communication_pid[resistance_group_to_meet][0];

                char temp[20];
                snprintf(temp, sizeof(temp), "%d", resistance_group_pid);
                send_info_to_gui(fifo_gui, CITIZEN_MEET_RESISTANT_GROUP_START, getpid(), temp);

                char fifo_name[256];
                snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_for_resistance_group_%d", resistance_group_pid);
                int fifo_fd = open(fifo_name, O_WRONLY);
                Information info;
                info.id = getpid();
                int time_spent = rand() % 4 + 1;
                snprintf(info.content, sizeof(info.content), "%d", time_spent);
                write(fifo_fd, &info, sizeof(Information));

                printf("============== kill %d\n", resistance_group_pid);
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
                sleep(rand() % 3 + 2);
                send_info_to_gui(fifo_gui, CITIZEN_MEET_RESISTANT_GROUP_END, getpid(), temp);
                shared_data->communication_pid[resistance_group_to_meet][2]--;
            }
        }
    }


    return 0;
}


