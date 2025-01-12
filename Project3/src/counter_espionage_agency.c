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
    int creation_time;
    int is_killed_caught_counter_espionage_agency;
} Member;
Config config;
Member *members;

ListPtr citizen_list;
SharedMemory *shared_data;
key_t key;
int msgid;
int fifo_gui;
int status = 1;



// user defined threshold after questioning

void *counter_espionage_agency_member(void *arg);

void *read_from_queue(void *arg);

void check_members_after_attack(Message_Queue msg);

void handle_SIGUSR1();

void handle_SIGUSR2();

void regular_analyzes();


int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    shared_data = (SharedMemory *) shmat(shm_id, NULL, 0);
    config = shared_data->config;
    citizen_list = NULL;

    key = ftok("/tmp/citizen_list_queue", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);
    fifo_gui = open(FIFO_NAME, O_WRONLY);

    pthread_t queue_thread;
    // create single thread to read from message queue
    if (pthread_create(&queue_thread, NULL, read_from_queue, NULL) != 0) {
        perror("Thread creation failed");
        return EXIT_FAILURE;
    }


    signal(SIGUSR1, handle_SIGUSR1);
    signal(SIGUSR2, handle_SIGUSR2);
    members = (Member *) malloc(config.number_of_member_in_counter_espionage_agency * sizeof(Member));


    int args[config.number_of_member_in_counter_espionage_agency];

    for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {
        args[i] = i;
        members[i].creation_time = time(NULL);
        if (pthread_create(&members[i].thread_id, NULL, counter_espionage_agency_member, &args[i]) != 0) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
        }
    }


    while (shared_data->simulation_status) {
        for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {

            pthread_join(members[i].thread_id, NULL);
            if (members[i].is_killed_caught_counter_espionage_agency == 1) {

            }

        }
    }
    pthread_join(queue_thread, NULL);
    free(members);


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


void handle_SIGUSR2() {
    for (int i = 0; i < config.number_of_member_in_counter_espionage_agency; i++) {
        if ((time(NULL) - members[i].creation_time) > 10) {
            int state = rand() % 3; // 0-2 kill, caught,normal
            if (state == 0 || state == 1) {
                members[i].is_killed_caught_counter_espionage_agency = 1;
            }

        }

    }


}

void *counter_espionage_agency_member(void *arg) {
    int id = *(int *) arg;

    int created_time = time(NULL);
    printf("Counter espionage agency member %d started\n", id);
    while (shared_data->simulation_status) {
        int time_spent = time(NULL) - created_time;
        if (time_spent > 2) {
            regular_analyzes();
        }
        if (members[id].is_killed_caught_counter_espionage_agency == 1) {
            printf("Counter espionage agency member %d is killed or caught\n", id);
            break;
        }
    }


    printf("Counter espionage agency member %d finished\n", id);
    return NULL;
}

void *read_from_queue(void *arg) {
    Message_Queue msg;
    while (status) {
        if (msgrcv(msgid, &msg, sizeof(msg), 1, 0) > 0) {
            printf("Counter espionage agency: Received message from citizen\n");
            printf("Group ID: %d, Citizen ID: %d, Time Spent: %d\n", msg.group_id, msg.citizen_id, msg.time_spent);

            send_info_to_gui(fifo_gui, COUNTER_ESPIONAGE_LOG_ABOUT_CITIZEN, msg.citizen_id, "");
            appendNode(&citizen_list, msg.group_id, msg.citizen_id, msg.time_spent);
        }
        if (msgrcv(msgid, &msg, sizeof(msg), 2, 0) > 0) {
            printf("Counter espionage agency: Received message from attack\n");
            check_members_after_attack(msg);


        }
    }


    return NULL;
}


void check_members_after_attack(Message_Queue msg) {
    int the_spy = -1;
    int member_number = 0;

    char *suspicion_rate = strtok(msg.suspicion_rates, " ");

    while (suspicion_rate != NULL) {

        if (atoi(suspicion_rate) > config.suspicion_rate_threshold) {
            int randNum = rand() % 100;
            if (randNum >= 80) {
                the_spy = member_number;
                break;

            }
        }
        member_number++;
        suspicion_rate = strtok(NULL, " ");
    }
    Message_Queue msg1;
    msg1.message_type = 3;
    msg1.the_spy = the_spy;
    msgsnd(msgid, &msg1, sizeof(msg1), 0);

}


void regular_analyzes() {
    Node *temp = citizen_list;
    int counts[1000] = {0};  // assuming there are no more than 1000 unique citizen IDs

    while (temp != NULL) {
        counts[temp->citizen_id]++;
        temp = temp->next;
    }

    for (int i = 0; i < 1000; i++) {
        if (counts[i] == 10) {
            printf("Citizen with ID %d has appeared 10 times.\n", i);
        }
    }
}

