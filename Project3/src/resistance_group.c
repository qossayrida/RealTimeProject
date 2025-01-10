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
ListPtr citizen_list;
key_t key;
int msgid;
int my_type;


void* resistant_group_member(void* arg);

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
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    config = shared_data->config;
    citizen_list = NULL;


    key = ftok("/tmp/citizen_list_queue", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);


    Member members[number_of_member_in_resistant_group];

    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        members[i].id = i;
        if (pthread_create(&members[i].thread_id, NULL, resistant_group_member, &members[i].id ) != 0) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
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


void* resistant_group_member(void* arg) {
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

