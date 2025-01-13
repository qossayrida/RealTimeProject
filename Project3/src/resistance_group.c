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
#include "sem.h"
#include "linked_list.h"

typedef struct {
    pthread_t thread_id;
    int id;
    int state_after_attack;
    int suspicion_rate;
} Member;

typedef struct {
    int id;
    int sleep_time;
} Argument;

Config config;
SharedMemory *shared_data;
Member *members;
key_t key;
int msgid;
int my_type;
int fifo_fd;
int go_out;
int spy_index;
int sem;
int fifo_gui;
int spy_detected_by_counter_espionage_agency;
int number_of_member_in_resistant_group;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int proceed = 0;
char fifo_name[256];
char SUSPICION_RATES[SIZE_OF_STATE];


void *normal_member(void *arg);

void *spy_member(void *arg);

int get_enemy_id();

void handle_SIGUSR1();

void handle_SIGUSR2();

int get_state(int id);

void send_state_after_attack_to_gui();


int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <type> <number_of_members> <pipe>\n", argv[0]);
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

    number_of_member_in_resistant_group = atoi(argv[2]);
    printf("Resistant group: Starting\n");
    printf("Resistant group: Type %d\n", my_type);
    printf("Resistant group: Number of members %d\n", number_of_member_in_resistant_group);


    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    shared_data = (SharedMemory *) shmat(shm_id, NULL, 0);
    sem = semget(1234, 1, 0666);
    config = shared_data->config;
    spy_detected_by_counter_espionage_agency = -1;


    key = ftok("/tmp/citizen_list_queue", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);


    pid_t pid = getpid();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_for_resistance_group_%d", pid);
    if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }
    fifo_fd = open(fifo_name, O_RDONLY | O_NONBLOCK);

    char *pipe_fd_str = argv[3];

    fifo_gui = atoi(pipe_fd_str);
    if (fifo_gui <= 0) {
        fprintf(stderr, "Invalid pipe file descriptor\n");
        exit(EXIT_FAILURE);
    }


    printf("Resistant group: FIFO created\n");
    signal(SIGUSR1, handle_SIGUSR1);
    signal(SIGUSR2, handle_SIGUSR2);

    members = (Member *) malloc(number_of_member_in_resistant_group * sizeof(Member));
    Argument *args = (Argument *) malloc(number_of_member_in_resistant_group * sizeof(Argument));

    // choose one member to be spy
    spy_index = rand() % number_of_member_in_resistant_group;
    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        char result[40];
        if(i==spy_index){
        snprintf(result, sizeof(result), "%s %d", "Spy", i);
        } else{
        snprintf(result, sizeof(result), "%s %d", "Normal", i);
        }
        send_info_to_gui(fifo_gui, RESISTANT_GROUP_MEMBER_CREATED, getpid(), result);

        args[i].id = i;
        args[i].sleep_time = 0;
        if (i == spy_index) {
            if (pthread_create(&members[i].thread_id, NULL, spy_member, &args[i]) != 0) {
                perror("Thread creation failed");
                return EXIT_FAILURE;
            }
        } else {
            if (pthread_create(&members[i].thread_id, NULL, normal_member, &args[i]) != 0) {
                perror("Thread creation failed");
                return EXIT_FAILURE;
            }
        }
    }

    shared_data->communication_pid[shared_data->array_counter][2]++;

    while (shared_data->simulation_status) {

        for (int i = 0; i < number_of_member_in_resistant_group; i++) {
            if (i == spy_detected_by_counter_espionage_agency) {
                continue;
            }

            pthread_join(members[i].thread_id, NULL);
            if (members[i].state_after_attack == KILLED ||
                members[i].state_after_attack == CAUGHT ||
                members[i].state_after_attack == HEALTHY ||
                members[i].state_after_attack == INJURED_SEVERELY) {

                args[i].id = i;
                args[i].sleep_time = 0;
                if (pthread_create(&members[i].thread_id, NULL, normal_member, &args[i]) != 0) {
                    perror("Thread creation failed");
                    return EXIT_FAILURE;
                }

            } else if (members[i].state_after_attack == INJURED_LIGHTLY) {
                args[i].id = i;
                args[i].sleep_time = rand() % 2;
                if (pthread_create(&members[i].thread_id, NULL, spy_member, &args[i]) != 0) {
                    perror("Thread creation failed");
                    return EXIT_FAILURE;
                }
            }
        }
    }

    printf("----------------> i received signal %d\n",shared_data->simulation_status);
    if ((unlink(fifo_name)) == 0) {
        printf("Resistant group %d: fifo has been removed\n", getpid());
    }
    free(members);
    printf("Resistant group %d: Terminating\n", getpid());
    return 0;
}

void handle_SIGUSR1() {
    printf("----------------> i received signal %d\n",shared_data->simulation_status);
    fflush(stdout);
    if (shared_data->simulation_status) {
        Information info;
        read(fifo_fd, &info, sizeof(Information));

        Message_Queue msg;
        msg.message_type = 1;
        msg.group_id = getpid();
        msg.citizen_id = info.id;
        msg.time_spent = atoi(info.content);
        msgsnd(msgid, &msg, sizeof(msg), 0);
    } else {
        if ((unlink(fifo_name)) == 0) {
            printf("Resistant group %d: fifo has been removed\n", getpid());
        }
        free(members);
        printf("Resistant group %d: Terminating\n", getpid());
        exit(EXIT_SUCCESS);
    }
}

void handle_SIGUSR2() {
    memset(SUSPICION_RATES, 0, sizeof(SUSPICION_RATES));
    for (int i = 0; i < number_of_member_in_resistant_group; i++) {

        int state = rand() % 5; // 0-4 kill, caught, injured severely, injured lightly and normal
        if (i == spy_index) {
            state = rand() % 2 == 0 ? HEALTHY : INJURED_LIGHTLY;
        }
        members[i].state_after_attack = state;

        sem_wait(sem);
        if (state == KILLED) {
            shared_data->number_of_killed_resistant_group++;
        } else if (state == INJURED_SEVERELY || state == INJURED_LIGHTLY) {
            shared_data->number_of_injured_resistant_group++;
        }

        sem_post(sem);
        char temp[10];
        sprintf(temp, "%d", members[i].suspicion_rate);  // Convert the number to string
        strcat(SUSPICION_RATES, temp);    // Append the number to the result
        strcat(SUSPICION_RATES, " ");
    }
    send_state_after_attack_to_gui();


    pthread_mutex_lock(&mutex);
    proceed = 1;
    pthread_cond_broadcast(&cond); // Wake up all waiting threads
    pthread_mutex_unlock(&mutex);
    go_out = 1;


    Message_Queue msg;
    msg.message_type = 2;
    msg.group_id = getpid();
    snprintf(msg.suspicion_rates, sizeof(msg.suspicion_rates), "%s", SUSPICION_RATES);
    msgsnd(msgid, &msg, sizeof(msg), 0);

    if (msgrcv(msgid, &msg, sizeof(msg), 3, 0) > 0) {
        if (msg.the_spy != -1) {
            if (spy_detected_by_counter_espionage_agency != -1) {
                char spy_number[10];
                sprintf(spy_number, "%d", spy_detected_by_counter_espionage_agency);
                send_info_to_gui(fifo_gui, COUNTER_ESPIONAGE_AGENCY_DETECTED_SPY, getpid(), spy_number);
                spy_detected_by_counter_espionage_agency = msg.the_spy;
            }
        }
    }
}

void *normal_member(void *arg) {
    Argument *argument = (Argument *) arg;

    if (argument->sleep_time > 0) {
        sleep(argument->sleep_time);
        char temp[20];
        snprintf(temp, sizeof(temp), "%d", argument->id);
        send_info_to_gui(fifo_gui, RESISTANT_GROUP_RETURN_FROM_INJURED, getpid(), temp);
    }

    printf("Resistant group member %d started\n", argument->id);
    pthread_mutex_lock(&mutex);

    while (!proceed) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);


    printf("Resistant group member %d finished\n", argument->id);
    return NULL;
}


void *spy_member(void *arg) {
    Argument *argument = (Argument *) arg;
    int created_time = time(NULL);
    printf("Spy member %d started\n", argument->id);

    char fifo_name[256];
    int pid = get_enemy_id();
    snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo_to_write_at_enemy_%d", pid);
    int fifo_fd = open(fifo_name, O_WRONLY);


    while (shared_data->simulation_status) {
        int time_spent = time(NULL) - created_time;
        if (time_spent > 2) {
            printf("Spy member %d increase 2 second\n", argument->id);
            Information info;
            info.type = RESISTANT_GROUP;
            snprintf(info.content, sizeof(info.content), "%d", time_spent);
            write(fifo_fd, &info, sizeof(Information));
            created_time = time(NULL);
        }
        if (go_out) {
            break;
        }
    }

    go_out = 0;

    printf("Spy member %d finished\n", argument->id);
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

void send_state_after_attack_to_gui() {
    char state_str[20];
    memset(state_str, 0, sizeof(state_str));
    for (int i = 0; i < number_of_member_in_resistant_group; i++) {
        if (members[i].state_after_attack == KILLED) {
            strcat(state_str, "0 ");
        } else if (members[i].state_after_attack == CAUGHT) {
            strcat(state_str, "1 ");
        } else if (members[i].state_after_attack == INJURED_SEVERELY) {
            strcat(state_str, "2 ");
        } else if (members[i].state_after_attack == INJURED_LIGHTLY) {
            strcat(state_str, "3 ");
        } else {
            strcat(state_str, "4 ");
        }
    }

    send_info_to_gui(fifo_gui, RESISTANT_GROUP_STATE_AFTER_ATTACK, getpid(), state_str);
}

