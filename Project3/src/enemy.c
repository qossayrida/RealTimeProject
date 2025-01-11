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

void handle_SIGUSR1();

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <pid_for_resistance_group> <number_of_members>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int pid_for_resistance_group = atoi(argv[1]);
    int number_of_member_in_resistant_group = atoi(argv[2]);

    printf("Counter espionage agency: PID for resistance group %d\n", pid_for_resistance_group);
    printf("Counter espionage agency: Number of members %d\n", number_of_member_in_resistant_group);

    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    Config config = shared_data->config;

    char fifo_name[256];
    pid_t pid = getpid();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_to_write_at_enemy_%d", pid);
    if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }
    int fifo_fd = open(fifo_name, O_RDONLY | O_NONBLOCK);


    shared_data->communication_pid[shared_data->array_counter++][2]++;

    int probability_to_attack = 0;
    while (shared_data->simulation_status) {
        Information info;
        if (read(fifo_fd, &info, sizeof(Information)) > 0) {

            if (info.type == RESISTANT_GROUP) {
                printf("ENEMY: From RESISTANT_GROUP Received information Time spent = %s \n", info.content);
                int time_spent = atoi(info.content);

                // modify probability to attack use suitable formula
                probability_to_attack += time_spent;
            }else if (info.type == CITIZEN) {
                printf("ENEMY: From CITIZEN Received information Time spent = %s \n", info.content);
                int time_spent = atoi(info.content);

                // modify probability to attack use suitable formula
                probability_to_attack += time_spent;
            }

            if (probability_to_attack > config.min_probability_to_attack) {
                printf("ENEMY: Attack to resistance group\n");
                kill(pid_for_resistance_group, SIGUSR2);
                probability_to_attack = 0;
            }
        }
    }

    close(fifo_fd);
    if ( (unlink(fifo_name)) == 0 ) {
        printf("fifo: fifo has been removed for enemy %d\n",getpid());
    }
    return 0;
}

void handle_SIGUSR1() {
    printf("Counter espionage agency: Received SIGUSR1\n");
}
