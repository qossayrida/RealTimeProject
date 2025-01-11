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
#include <pthread.h>
#include "config.h"
#include "linked_list.h"

typedef struct {
    pthread_t thread_id;
    int id;
    int suspicion_rate;
} Member;

Config config;
SharedMemory *shared_data;
key_t key;
int msgid;
int my_type;
int fifo_fd;

void* normal_member(void* arg);
void* spy_member(void* arg);
int get_enemy_id();


int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <type> <number_of_members>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "0") == 0) {
        my_type = Social_Type;
    } else if (strcmp(argv[1], "1") == 0) {
        my_type = Military_Type;
    } else {
        fprintf(stderr, "Invalid type\n");
        return EXIT_FAILURE;
    }

    int number_of_member_in_resistant_group = atoi(argv[2]);
    printf("Resistant group: Starting\n");
    printf("Resistant group: Type %d\n", my_type);
    printf("Resistant group: Number of members %d\n", number_of_member_in_resistant_group);


    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    config = shared_data->config;
    print_config(&config);


    key = ftok("/tmp/citizen_list_queue", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    char fifo_name[256];
    pid_t pid = getpid();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", pid);
    if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }
    fifo_fd = open(fifo_name, O_WRONLY);

    signal(SIGUSR1, handle_SIGUSR1);


    Member members[number_of_member_in_resistant_group];

    // choose one member to be spy
    int spy_index = rand() % number_of_member_in_resistant_group;
    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        members[i].id = i;
        if (i == spy_index) {
            if (pthread_create(&members[i].thread_id, NULL, spy_member, &members[i].id) != 0) {
                perror("Thread creation failed");
                return EXIT_FAILURE;
            }
        } else {
            if (pthread_create(&members[i].thread_id, NULL, normal_member, &members[i].id) != 0) {
                perror("Thread creation failed");
                return EXIT_FAILURE;
            }
        }
    }

    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        pthread_join(members[i].thread_id, NULL);
    }

    printf("Resistant group: Terminating\n");
    return 0;
}

void handle_SIGUSR1() {
    Information info;
    int time_spent = rand() % 4 + 1;
    snprintf(info.content, sizeof(info.content), "%d",time_spent);
    write(fifo_fd, &info, sizeof(Information));

    read()

    Message_Queue msg;
    msg.message_type = 1;
    msg.group_id = getpid();
    msg.citizen_id = id;
    msg.time_spent = time_spent;
    msgsnd(msgid, &msg, sizeof(msg), 0);
}

void* normal_member(void* arg) {
    int id = *(int*)arg;
    printf("Resistant group member %d started\n", id);

    // pause for random time

    printf("Resistant group member %d finished\n", id);
    return NULL;
}

void* spy_member(void* arg) {
    int id = *(int*)arg;
    int created_time = time(NULL);
    printf("Spy member %d started\n", id);

    char fifo_name[256];
    int pid = get_enemy_id();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", pid);
    if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }
    int fifo_fd = open(fifo_name, O_WRONLY);


    while (1){
        int time_spent = time(NULL) - created_time;
        if (time_spent > 2) {
            Information info;
            info.type = RESISTANT_GROUP;
            snprintf(info.content, sizeof(info.content), "%d",time_spent);
            write(fifo_fd, &info, sizeof(Information));
        }
    }

    printf("SPY member %d finished\n", id);
    return NULL;
}


int get_enemy_id() {
    pid_t pid = getpid();
    for (int i = 0; i < shared_data->array_counter; i++) {
        if (shared_data->communication_pid[i][0] == pid) {
            return shared_data->communication_pid[i][1];
        }
    }
    return -1;
}