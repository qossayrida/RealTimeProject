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

Config config;
Information info_buffer[50];
SharedMemory *shared_data;
int shm_id;
int fifo_fd;
pid_t counter_espionage_agency_pid;

void handle_alarm();
pid_t create_counter_espionage_agency(const char *program, int shm_id);
void create_citizens(int shm_id);
void create_resistant_group_and_enemy(int shm_id,int type);

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

    counter_espionage_agency_pid = create_counter_espionage_agency("./bin/counter_espionage_agency", shm_id);
    create_citizens(shm_id);
    create_resistant_group_and_enemy(shm_id,  Social_Type);


    signal(SIGALRM, handle_alarm);
    alarm(config.time_create_resistant_group);


    if (mknod(FIFO_NAME, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }

    fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);


    while (1){
        int bytes_read = read(fifo_fd, info_buffer, sizeof(info_buffer));
        int num_info = (bytes_read != -1) ? bytes_read / sizeof(Information) : 0;

        if (num_info > 0) {
            printf("Timer: Received %d information.\n", num_info);
        }

        for (int i = 0; i < num_info; i++) {
            switch (info_buffer[i].type) {
                case COUNTER_ESPIONAGE_AGENCY:
                    printf("Main: Received information from counter espionage agency: %s\n", info_buffer[i].content);
                    break;
                case RESISTANT_GROUP:
                    printf("Main: Received information from resistant group: %s\n", info_buffer[i].content);
                    break;
                case ENEMY:
                    printf("Main: Received information from enemy: %s\n", info_buffer[i].content);
                    break;
                case CITIZEN:
                    printf("Main: Received information from citizen: %s\n", info_buffer[i].content);
                    break;
            }
        }
    }

    printf("Main: exit\n");
    return 0;
}

void handle_alarm() {
    printf("Main: Creating new resistant group\n");
    create_resistant_group_and_enemy(shm_id,  Social_Type);
    kill(counter_espionage_agency_pid, SIGUSR1);
    close(fifo_fd);
    if ( (unlink(FIFO_NAME)) == 0 ) {
        printf("fifo: fifo has been removed\n");
        exit(0);
    }
    exit(0);
}


pid_t create_counter_espionage_agency(const char *program, int shm_id) {
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

    return pid;
}

void create_citizens(int shm_id) {
    srand(time(NULL));
    int number_of_citizen = rand() % (config.max_number_of_citizen - config.min_number_of_citizen + 1) + config.min_number_of_citizen;
    int percentage_of_spy = 30;

    // Create citizens process with random type (spy or normal)
    for (int i = 0; i < number_of_citizen; i++) {
        int type = (rand() % 100 < percentage_of_spy) ? SPY : NORMAL;
        pid_t pid = fork();
        if (pid == 0) { // Child process
            char shm_id_str[10];
            char type_str[10];
            sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
            setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
            sprintf(type_str, "%d", type);
            execlp("./bin/citizen", "./bin/citizen", type_str, NULL);
            perror("execlp failed");
            exit(1); // Exit if execlp fails
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
    }
}

void create_resistant_group_and_enemy(int shm_id,int type) {

    srand(time(NULL));
    int number_of_member_in_resistant_group = rand() % (config.max_number_member_in_resistant_group - config.min_number_member_in_resistant_group + 1) + config.min_number_member_in_resistant_group;

    pid_t pid = fork();
    shared_data->communication_pid[shared_data->array_counter][0] = pid;
    if (pid == 0) { // Child process
        char shm_id_str[10];
        char type_str[10];
        char number_of_member_in_resistant_group_str[10];
        sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
        setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
        sprintf(type_str, "%d", type);
        sprintf(number_of_member_in_resistant_group_str, "%d", number_of_member_in_resistant_group);
        execlp("./bin/resistance_group", "./bin/resistance_group", type_str,number_of_member_in_resistant_group_str, NULL);
        perror("execlp failed");
        exit(1); // Exit if execlp fails
    } else if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    pid_t enemy_pid = fork();
    shared_data->communication_pid[shared_data->array_counter++][1] = enemy_pid;
    if (enemy_pid==0) {
        char shm_id_str[10];
        char pid_str[20];
        char number_of_member_in_resistant_group_str[10];
        sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
        setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
        snprintf(pid_str, sizeof(pid_str), "%d", pid);
        sprintf(number_of_member_in_resistant_group_str, "%d", number_of_member_in_resistant_group);
        execlp("./bin/enemy", "./bin/enemy",pid_str,number_of_member_in_resistant_group_str, NULL);
        perror("execlp failed");
        exit(1); // Exit if execlp fails
    } else if (enemy_pid < 0) {
        perror("Fork failed");
        exit(1);
    }

}