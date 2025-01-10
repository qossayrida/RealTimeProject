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
ListPtr citizen_list;
key_t key;
int msgid;
int my_type;


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
    citizen_list = NULL;


    key = ftok("/tmp/citizen_list_queue", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);


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

    printf("Resistant group List:\n");
    printList(citizen_list);
    freeList(citizen_list);

    printf("Resistant group: Terminating\n");
    return 0;
}


void* normal_member(void* arg) {
    int id = *(int*)arg;
    printf("Resistant group member %d started\n", id);
    appendNode(&citizen_list,id,(id+1)*10);

    Message_Queue msg;
    msg.message_type = 1;
    msg.group_id = getpid();
    msg.citizen_id = id;
    msg.time_spent = (id+1)*10;
    msgsnd(msgid, &msg, sizeof(msg), 0);

    printf("Resistant group member %d finished\n", id);
    return NULL;
}

void* spy_member(void* arg) {
    int id = *(int*)arg;
    printf("Spy member %d started\n", id);
    appendNode(&citizen_list,id,(id+1)*10);

    char fifo_name[256];
    int pid = get_enemy_id();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_%d", pid);
    if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }
    int fifo_fd = open(fifo_name, O_RDONLY | O_NONBLOCK);


    Message_Queue msg;
    msg.message_type = 1;
    msg.group_id = getpid();
    msg.citizen_id = id;
    msg.time_spent = (id+1)*10;
    msgsnd(msgid, &msg, sizeof(msg), 0);

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