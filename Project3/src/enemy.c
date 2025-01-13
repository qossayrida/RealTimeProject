#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include "config.h"

Information info_buffer[50];
int fifo_gui;

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <pid_for_resistance_group> <number_of_members>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int pid_for_resistance_group = atoi(argv[1]);
    int number_of_member_in_resistant_group = atoi(argv[2]);
    int type = atoi(argv[3]);

    printf("Enemy: PID for resistance group %d\n", pid_for_resistance_group);
    printf("Enemy: Number of members %d\n", number_of_member_in_resistant_group);

    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *) shmat(shm_id, NULL, 0);
    Config config = shared_data->config;

    char fifo_name[256];
    pid_t pid = getpid();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_to_write_at_enemy_%d", pid);
    if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }
    int fifo_fd = open(fifo_name, O_RDONLY | O_NONBLOCK);

    char *pipe_fd_str = getenv("PIPE_FD");
    if (pipe_fd_str == NULL) {
        fprintf(stderr, "PIPE_FD environment variable not set\n");
        exit(EXIT_FAILURE);
    }

    fifo_gui = atoi(pipe_fd_str);
    if (fifo_gui <= 0) {
        fprintf(stderr, "Invalid pipe file descriptor\n");
        exit(EXIT_FAILURE);
    }



    char result[40];
    if(type == For_Resistant_Group){
        snprintf(result, sizeof(result), "%s %d", "For_Resistant", pid_for_resistance_group);
    } else{
        snprintf(result, sizeof(result), "%s 0","For_Espionage");
    }
    send_info_to_gui(fifo_gui, ENEMY_CREATED, getpid(), result);

    if (type == For_Resistant_Group) {
        shared_data->communication_pid[shared_data->array_counter++][2]++;
    }

    int probability_to_attack = 0;
    while (shared_data->simulation_status) {
        if (type == For_Resistant_Group) {

            int bytes_read = read(fifo_fd, info_buffer, sizeof(info_buffer));
            int num_info = (bytes_read != -1) ? bytes_read / sizeof(Information) : 0;


            for (int i = 0; i < num_info; i++) {

                if (info_buffer[i].type == RESISTANT_GROUP) {
                    send_info_to_gui(fifo_gui, ENEMY_RECEIVE_FROM_RESISTANT_GROUP, getpid(), info_buffer[i].content);
                    int time_spent = atoi(info_buffer[i].content);

                    // modify probability to attack use suitable formula
                    probability_to_attack += time_spent;
                } else if (info_buffer[i].type == CITIZEN) {
                    char result[40];
                    snprintf(result, sizeof(result), "%s %d", info_buffer[i].content, info_buffer[i].id);
                    send_info_to_gui(fifo_gui, ENEMY_RECEIVE_FROM_CITIZEN, getpid(), result);

                    int time_spent = atoi(info_buffer[i].content);

                    // modify probability to attack use suitable formula
                    probability_to_attack += time_spent;
                }

                if (probability_to_attack > config.min_probability_to_attack) {
                    send_info_to_gui(fifo_gui, ENEMY_ATTACK_RESISTANT_GROUP, getpid(), "make pro=0");
                    kill(pid_for_resistance_group, SIGUSR2);
                    probability_to_attack = 0;
                }
            }

        } else if (type == For_Espionage_Agency) {

            int created_time = time(NULL);
            printf("Counter espionage Enemy started\n");

            while (shared_data->simulation_status) {
                int time_spent = time(NULL) - created_time;
                if (time_spent > 10) {
                    kill(pid_for_resistance_group, SIGUSR2);
                    send_info_to_gui(fifo_gui, ENEMY_ATTACK_COUNTER_ESPIONAGE_AGENCY, getpid(), "");
                    created_time = time(NULL);
                }
            }
        }
    }

    close(fifo_fd);
    if ((unlink(fifo_name)) == 0) {
        printf("fifo: fifo has been removed for enemy %d\n", getpid());
    }
    return 0;
}
