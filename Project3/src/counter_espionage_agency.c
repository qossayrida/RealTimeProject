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

Config config;
ListPtr citizen_list;
key_t key;
int msgid;
int status = 1;


// user defined threshold after questioning

void* counter_espionage_agency_member(void* arg);
void *read_from_queue(void *arg);
void handle_SIGUSR1();


int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    SharedMemory *shared_data = (SharedMemory *)shmat(shm_id, NULL, 0);
    config = shared_data->config;
    citizen_list = NULL;

    key = ftok("/tmp/citizen_list_queue", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);


    pthread_t queue_thread;
    // create single thread to read from message queue
    if (pthread_create(&queue_thread, NULL, read_from_queue, NULL) != 0) {
        perror("Thread creation failed");
        return EXIT_FAILURE;
    }

    signal(SIGUSR1, handle_SIGUSR1);

    pthread_t threads[config.number_of_member_in_counter_espionage_agency];
    int args[config.number_of_member_in_counter_espionage_agency];

    for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {
        args[i] = i;
        if (pthread_create(&threads[i], NULL, counter_espionage_agency_member, &args[i]) != 0) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(queue_thread, NULL);


    printf("Counter espionage agency: Terminating\n");
    return 0;
}

void handle_SIGUSR1() {
    status = 0;
    printList(citizen_list);
    freeList(citizen_list);

    msgctl(msgid, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

void* counter_espionage_agency_member(void* arg) {
    int id = *(int*)arg;
    printf("Counter espionage agency member %d started\n", id);
    printf("Counter espionage agency member %d finished\n", id);
    return NULL;
}

void *read_from_queue(void *arg) {
    Message_Queue msg;
    while(status){
        if (msgrcv(msgid, &msg, sizeof(msg), 1,0) > 0) {
            printf("Counter espionage agency: Received message from citizen\n");
            printf("Group ID: %d, Citizen ID: %d, Time Spent: %d\n", msg.group_id, msg.citizen_id, msg.time_spent);
            appendNode(&citizen_list,msg.group_id,msg.citizen_id,msg.time_spent);
        }
    }
    return NULL;
}