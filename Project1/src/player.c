#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "player.h"
#include "config.h"

int global_team_id,global_player_id;
int global_fifo_to_highest,global_fifo_to_lowest;
int *global_pipe_read_fd;
int is_turn = 0;  // Flag to check if the player is allowed to jump
int energy, is_will_pull_alone=0;
int is_second_player_leader = 0;
char message_fifo[2][50];
char result[50];

Config *global_config;

void handle_player_SIGUSR1();
void handle_player_SIGUSR2();
void handle_pull_alone();
void jump(int team_id, int player_id, Config *config);
void pull_leader(Config *config);
void pull_non_leader(Config *config);
void pull_alone();
int generate_random_energy();
void send_message(int type, char *content);

void player_main(int team_id, int player_id, Config *config, int *pipe_read_fd, int fifo_to_highest, int fifo_to_lowest){
    global_team_id = team_id;
    global_player_id = player_id;
    global_pipe_read_fd = pipe_read_fd;
    global_fifo_to_highest = fifo_to_highest;
    global_fifo_to_lowest = fifo_to_lowest;
    global_config = config;

    srand(time(NULL) ^ (getpid() << 16));

    energy = generate_random_energy();  // Adjust based on team

    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, handle_player_SIGUSR1);
    signal(SIGUSR2, handle_player_SIGUSR2);
    signal(SIGQUIT, handle_pull_alone);


    time_t previas_time = time(NULL);

    while (1) {

        if (time(NULL) - previas_time > 5) {
            previas_time = time(NULL);
            energy = (energy != 0) ? (energy - 1) : 0;

            memset(result, '\0', sizeof(result));
            snprintf(result, sizeof(result), "%d",energy);
            send_message(ENERGY_ID,result);
        }

        // Wait until it’s this player’s turn (indicated by `is_turn` flag)
        while (!is_turn) {
            pause();  // Wait for the signal to start the jump
        }

        if (is_turn == 2) {
            if (is_second_player_leader == 1){
                printf("Team %d: LEADER is PLAYER 2\n", team_id + 1);
                fflush(stdout);
                pull_leader(config);
                is_second_player_leader = 0;
            }else if (player_id==2){
                printf("Team %d: LEADER is PLAYER 3\n", team_id + 1);
                fflush(stdout);
                pull_leader(config);
            }else{
                pull_non_leader(config);
            }
        } else if (is_turn == 3) {
            pull_alone(config);
        }
    }
}

void handle_player_SIGUSR1() {
    printf("Team %d: Received signal to jump from assistant referee\n", global_team_id + 1);
    fflush(stdout);
    jump(global_team_id, global_player_id, global_config);
}

void handle_player_SIGUSR2() {
    printf("Team %d: Received signal to pull from another player\n", global_team_id + 1);
    fflush(stdout);
    if(!is_will_pull_alone) {
        is_turn = 2;
    } else {
        is_turn = 3;
    }
}

void jump(int team_id, int player_id, Config *config) {
    if(player_id == 1){
        is_second_player_leader = 1;
    }

    int stabilization_time;

    stabilization_time = (rand() % (config->stabilization_time_max - config->stabilization_time_min))
                         + config->stabilization_time_min;
    printf("Team %d: Player %d jumping... Stabilization time: %d\n", team_id + 1, player_id + 1, stabilization_time);
    fflush(stdout);
    usleep(stabilization_time * 1000);

    // Decrease energy for jump and pull
    energy = (energy >= 10) ? (energy - 10) : 0;
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);

    send_message(JUMP_END_ID, "");
}


void pull_leader(Config *config) {
    int received_pull_time;

    // Read the pull time from the non-leader
    if (read(global_fifo_to_highest, message_fifo[0], sizeof(message_fifo[0])) > 0) {
        if (sscanf(message_fifo[0], "PULL_TIME %d", &received_pull_time) == 1) {
            printf("Team %d: Leader Player %d received pull time: %d\n", global_team_id + 1, global_player_id + 1, received_pull_time);

            // Calculate adjusted pull time based on an equation
            int adjusted_pull_time = received_pull_time + (1);  // Change it to the equation Ya MOHAMED FREED
            printf("Team %d: Leader Player %d calculated new pull time: %d\n", global_team_id + 1, global_player_id + 1, adjusted_pull_time);

            // Send the adjusted pull time back to the non-leader
            snprintf(message_fifo[1], sizeof(message_fifo[1]), "%d", adjusted_pull_time);
            if (write(global_fifo_to_lowest, message_fifo[1], strlen(message_fifo[1])) == -1) {
                perror("Failed to send adjusted pull time to FIFO");
            }

            usleep(adjusted_pull_time * 1000);
        }
    }

    // Energy check
    energy = (energy >= 10) ? (energy - 10) : 0;
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);

    send_message(PULL_END_ID, "");
}

void pull_non_leader(Config *config) {
    int pull_time = (rand() % (config->pull_time_max - config->pull_time_min)) + config->pull_time_min;

    // Send pull time to FIFO for leader to read
    snprintf(message_fifo[0], sizeof(message_fifo[0]), "PULL_TIME %d", pull_time);
    if (write(global_fifo_to_highest, message_fifo[0], strlen(message_fifo[0])) == -1) {
        perror("Failed to write pull time to FIFO");
    }

    // Wait to receive the adjusted pull time from the leader
    if (read(global_fifo_to_lowest, message_fifo[1], sizeof(message_fifo[1])) > 0) {
        int adjusted_pull_time = atoi(message_fifo[1]);
        printf("Team %d: Player %d received adjusted pull time: %d\n", global_team_id + 1, global_player_id + 1, adjusted_pull_time);
        usleep(adjusted_pull_time * 1000);
    }

    // Energy check
    energy = (energy >= 10) ? (energy - 10) : 0;
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);
}


void pull_alone(Config *config){
    int pull_time = (rand() % (config->pull_time_max - config->pull_time_min)) + config->pull_time_min;
    usleep(pull_time * 1000);

    energy = (energy >= 10) ? (energy - 10) : 0;


    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);


    send_message(PULL_END_ID, "");
}

void send_message(int type, char *content){
    Message msg;
    msg.type = type;
    msg.player_id = global_player_id;
    msg.team_id = global_team_id;
    snprintf(msg.content, sizeof(msg.content), "%s", content);
    if(write(global_pipe_read_fd[1], &msg, sizeof(Message))== -1)
        perror("Failed to write team data to referee");
}


void handle_pull_alone(){
    is_will_pull_alone = 1;
}


int generate_random_energy() {
    int min_energy, max_energy;

    switch (global_team_id) {
        case 0:  // Team A
            min_energy = global_config->initial_energy_min[0];
            max_energy = global_config->initial_energy_max[0];
            break;
        case 1:  // Team B
            min_energy = global_config->initial_energy_min[1];
            max_energy = global_config->initial_energy_max[1];
            break;
        case 2:  // Team C
            min_energy = global_config->initial_energy_min[2];
            max_energy = global_config->initial_energy_max[2];
            break;
        default:
            fprintf(stderr, "Invalid team ID\n");
            return -1;
    }

    return (rand() % (max_energy - min_energy + 1)) + min_energy;
}
