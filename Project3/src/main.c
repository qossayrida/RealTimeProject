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
#include "sem.h"
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

void create_resistant_group_and_enemy(int shm_id, int type);

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
    shared_data->array_counter = 0;
    shared_data->simulation_status = 1;

    // Initialize semaphore
    init_semaphore();

    counter_espionage_agency_pid = create_counter_espionage_agency("./bin/counter_espionage_agency", shm_id);
    create_resistant_group_and_enemy(shm_id, Social_Type);
    create_citizens(shm_id);

    signal(SIGALRM, handle_alarm);
    alarm(config.time_create_resistant_group);


    if (mknod(FIFO_NAME, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }

    fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);


    while (shared_data->simulation_status) {
        int bytes_read = read(fifo_fd, info_buffer, sizeof(info_buffer));
        int num_info = (bytes_read != -1) ? bytes_read / sizeof(Information) : 0;


        for (int i = 0; i < num_info; i++) {
            switch (info_buffer[i].type) {
                case COUNTER_ESPIONAGE_AGENCY:
                    printf("Main: Received information from counter espionage agency: %s\n", info_buffer[i].content);
                    break;
                case RESISTANT_GROUP_STATE_AFTER_ATTACK:
                    printf("Main: resistant group %d state after attack: %s\n", info_buffer[i].id,
                           info_buffer[i].content);
                    break;
                case ENEMY_RECEIVE_FROM_RESISTANT_GROUP:
                    printf("Main: enemy %d receive from resistance group : %s\n", info_buffer[i].id,
                           info_buffer[i].content);
                    break;
                case ENEMY_RECEIVE_FROM_CITIZEN:
                    printf("Main: enemy %d receive from citizen : %s\n", info_buffer[i].id, info_buffer[i].content);
                    break;
                case ENEMY_ATTACK_RESISTANT_GROUP:
                    printf("Main: enemy %d attack resistant group: %s\n", info_buffer[i].id, info_buffer[i].content);
                    break;
                case CITIZEN_CREATED:
                    printf("Main: citizen %d created: %s\n", info_buffer[i].id, info_buffer[i].content);
                    break;
            }

            if ((shared_data->number_of_killed_resistant_group >= config.killed_resistance_members_threshold) ||
                (shared_data->number_of_injured_resistant_group >= config.injured_resistance_members_threshold) ||
                (shared_data->number_of_killed_caught_counter_espionage_agency >=
                 config.killed_caught_counter_members_threshold)) {
                printf("Main: Resistance group has been killed\n");
                shared_data->simulation_status = 0;
            }
        }
    }

    printf("==> DEBUG from main\n");
    kill(counter_espionage_agency_pid, SIGUSR1);

    for (int i = 0; i < shared_data->array_counter; i++) {
        if (shared_data->communication_pid[i][2] == 3 || shared_data->communication_pid[i][2] == 2) {
            kill(shared_data->communication_pid[i][0], SIGUSR1);
        }
    }

    close(fifo_fd);
    if ((unlink(FIFO_NAME)) == 0) {
        printf("MAIN: fifo has been removed\n");
    }

    printf("Counter of array: %d\n", shared_data->array_counter);
    for (int i = 0; i < shared_data->array_counter; i++) {
        printf("Resistant group %d\n", shared_data->communication_pid[i][0]);
        printf("Enemy %d\n", shared_data->communication_pid[i][1]);
    }


    printf("Main: exit\n");
    return 0;
}

void handle_alarm() {
    printf("Main: **** Creating new resistant group ****\n");
    create_resistant_group_and_enemy(shm_id, Social_Type);
    alarm(config.time_create_resistant_group);
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


    pid_t enemy = fork();
    if (enemy == 0) { // Child process
        char shm_id_str[10];
        char pid_str[20];
        char number_of_member_in_counter_espionage_str[10];
        char type_str[10];
        sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
        sprintf(type_str, "%d", For_Espionage_Agency);
        setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
        snprintf(pid_str, sizeof(pid_str), "%d", pid);
        sprintf(number_of_member_in_counter_espionage_str, "%d", config.number_of_member_in_counter_espionage_agency);
        execlp("./bin/enemy", "./bin/enemy", pid_str, number_of_member_in_counter_espionage_str, type_str, NULL);
        perror("execlp failed");
        exit(1); // Exit if execlp fails
    } else if (enemy < 0) {
        perror("Fork failed");
        exit(1);
    }

    return pid;
}

void create_citizens(int shm_id) {
    srand(time(NULL));
    int number_of_citizen =
            rand() % (config.max_number_of_citizen - config.min_number_of_citizen + 1) + config.min_number_of_citizen;
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

void create_resistant_group_and_enemy(int shm_id, int type) {

    srand(time(NULL));
    int number_of_member_in_resistant_group =
            rand() % (config.max_number_member_in_resistant_group - config.min_number_member_in_resistant_group + 1) +
            config.min_number_member_in_resistant_group;
    pid_t pid = fork();
    if (pid == 0) { // Child process
        char shm_id_str[10];
        char type_str[10];
        char number_of_member_in_resistant_group_str[10];
        sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
        setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
        sprintf(type_str, "%d", type);
        sprintf(number_of_member_in_resistant_group_str, "%d", number_of_member_in_resistant_group);
        execlp("./bin/resistance_group", "./bin/resistance_group", type_str, number_of_member_in_resistant_group_str,
               NULL);
        perror("execlp failed");
        exit(1); // Exit if execlp fails
    } else if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    shared_data->communication_pid[shared_data->array_counter][0] = pid;

    pid_t enemy_pid = fork();
    if (enemy_pid == 0) {
        char shm_id_str[10];
        char pid_str[20];
        char number_of_member_in_resistant_group_str[10];
        char type_str[10];
        sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
        sprintf(type_str, "%d", For_Resistant_Group);
        setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
        snprintf(pid_str, sizeof(pid_str), "%d", pid);
        sprintf(number_of_member_in_resistant_group_str, "%d", number_of_member_in_resistant_group);
        execlp("./bin/enemy", "./bin/enemy", pid_str, number_of_member_in_resistant_group_str, type_str, NULL);
        perror("execlp failed");
        exit(1); // Exit if execlp fails
    } else if (enemy_pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    shared_data->communication_pid[shared_data->array_counter][1] = enemy_pid;
}