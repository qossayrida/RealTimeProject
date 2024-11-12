#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "config.h"

int team_id,player_id;
int global_fifo_to_highest,global_fifo_to_lowest;
int fifo_to_assistant_referee;
int is_turn = 0 ,is_first_time=1;  // Flag to check if the player is allowed to jump
int energy,max_energy,is_will_pull_alone=0;
int is_second_player_leader = 0;
char message_fifo[2][50];
char result[50];

Config config;

void handle_player_SIGUSR1();
void handle_player_SIGUSR2();
void handle_pull_alone();
void jump();
void pull_leader();
void pull_non_leader();
void pull_alone();
int generate_random_energy();
void send_message(int type, char *content);
void handle_alarm();
void busy_wait(int ms);
void update_energy(int time);

int main(int argc, char *argv[]) {
    if (argc < 7) {
        fprintf(stderr, "Usage: %s <id_player> <id_team> <fifo_name> <fifo_highest> <fifo_lowest> <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    player_id = atoi(argv[1]);
    team_id = atoi(argv[2]);
    global_fifo_to_highest =  open(argv[3], O_RDWR);
    global_fifo_to_lowest =  open(argv[4], O_RDWR);
    fifo_to_assistant_referee = open(argv[5], O_WRONLY);



    fflush(stdout);
    if (load_config(argv[6], &config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL) ^ (getpid() << 16));

    energy = generate_random_energy();  // Adjust based on team

    max_energy = energy;
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d", energy);
    send_message(READY_TO_PLAY, result);

    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, handle_player_SIGUSR1);
    signal(SIGUSR2, handle_player_SIGUSR2);
    signal(SIGQUIT, handle_pull_alone);
    signal(SIGALRM, handle_alarm);
    alarm(5);

    while (1) {
        // Wait until it’s this player’s turn (indicated by `is_turn` flag)
        while (!is_turn) {
            pause();  // Wait for the signal to start the jump
        }

        if (is_turn == 2) {
            if (is_second_player_leader == 1){
                pull_leader();
                is_second_player_leader = 0;
            }else if (player_id==2){
                fflush(stdout);
                pull_leader();
            }else{
                pull_non_leader();
            }
        } else if (is_turn == 3) {
            pull_alone();
        }

        is_turn = 0;
    }
}

void handle_player_SIGUSR1() {
    if (is_first_time && player_id == 0) {
        jump();
        is_first_time = 0;
    }
    else{
        int rest_time = (rand() % (config.rest_time_max - config.rest_time_min)) + config.rest_time_min;
        busy_wait(rest_time);
        jump();
    }
}

void handle_player_SIGUSR2() {
    if(!is_will_pull_alone) {
        is_turn = 2;
    } else {
        is_turn = 3;
    }
}

void jump() {

    send_message(JUMP_START_ID, "");
    if(player_id == 1){
        is_second_player_leader = 1;
    }

    int stabilization_time;

    stabilization_time = (rand() % (config.stabilization_time_max - config.stabilization_time_min))+config.stabilization_time_min;
    busy_wait(stabilization_time);

    // Decrease energy for jump and pull
    update_energy(stabilization_time);
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);

    send_message(JUMP_END_ID, "");
    is_turn = 1;
}


void pull_leader() {
    send_message(PULL_START_ID, "");
    int received_pull_time;
    double energy_decrease = 1+((max_energy-energy)/(double)max_energy);
    int pull_time = (int) (((rand() % (config.pull_time_max - config.pull_time_min)) + config.pull_time_min) * energy_decrease);

    // Read the pull time from the non-leader
    if (read(global_fifo_to_highest, message_fifo[0], sizeof(message_fifo[0])) > 0) {
        if (sscanf(message_fifo[0], "PULL_TIME %d", &received_pull_time) == 1) {

            // Calculate adjusted pull time based on an equation
            int adjusted_pull_time = 2.0/((1.0 /received_pull_time) + (1.0/pull_time));  // Change it to the equation Ya MOHAMED FREED

            // Send the adjusted pull time back to the non-leader
            snprintf(message_fifo[1], sizeof(message_fifo[1]), "%d", adjusted_pull_time);
            if (write(global_fifo_to_lowest, message_fifo[1], strlen(message_fifo[1])) == -1) {
                perror("Failed to send adjusted pull time to FIFO");
            }

            busy_wait(adjusted_pull_time);
        }
    }

    // Energy check
    update_energy(pull_time);
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);

    send_message(PULL_END_ID, "");
}

void pull_non_leader() {
    double energy_decrease = 1+((max_energy-energy)/(double)max_energy);
    int pull_time = (int) (((rand() % (config.pull_time_max - config.pull_time_min)) + config.pull_time_min) * energy_decrease);

    // Send pull time to FIFO for leader to read
    snprintf(message_fifo[0], sizeof(message_fifo[0]), "PULL_TIME %d", pull_time);
    if (write(global_fifo_to_highest, message_fifo[0], strlen(message_fifo[0])) == -1) {
        perror("Failed to write pull time to FIFO");
    }

    // Wait to receive the adjusted pull time from the leader
    if (read(global_fifo_to_lowest, message_fifo[1], sizeof(message_fifo[1])) > 0) {
        int adjusted_pull_time = atoi(message_fifo[1]);
//        printf("Team %d: Player %d received adjusted pull time: %d\n", team_id + 1, player_id + 1, adjusted_pull_time);
        busy_wait(adjusted_pull_time);
    }

    // Energy check
    update_energy(pull_time);
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);
}


void pull_alone(){
    send_message(PULL_START_ID, "");
    double energy_decrease = 1+((max_energy-energy)/(double)max_energy);
    int pull_time = (int) (((rand() % (config.pull_time_max - config.pull_time_min)) + config.pull_time_min) * energy_decrease);
    busy_wait(pull_time);

    update_energy(pull_time);
    memset(result, '\0', sizeof(result));
    snprintf(result, sizeof(result), "%d",energy);
    send_message(ENERGY_ID,result);


    send_message(PULL_END_ID, "");
}

void send_message(int type, char *content){
    Message msg;
    msg.type = type;
    msg.player_id = player_id;
    msg.team_id = team_id;
    snprintf(msg.content, sizeof(msg.content), "%s", content);
    if(write(fifo_to_assistant_referee, &msg, sizeof(Message))== -1)
        perror("Failed to write team data to referee");
}


void handle_pull_alone(){
    is_will_pull_alone = 1;
}


int generate_random_energy() {
    int min_energy, max_energy;

    switch (team_id) {
        case 0:  // Team A
            min_energy = config.initial_energy_min[0];
            max_energy = config.initial_energy_max[0];
            break;
        case 1:  // Team B
            min_energy = config.initial_energy_min[1];
            max_energy = config.initial_energy_max[1];
            break;
        case 2:  // Team C
            min_energy = config.initial_energy_min[2];
            max_energy = config.initial_energy_max[2];
            break;
        default:
            fprintf(stderr, "Invalid team ID\n");
            return -1;
    }

    return (rand() % (max_energy - min_energy + 1)) + min_energy;
}

void handle_alarm() {
    if (energy != 0) {
        energy--;
        //printf("%d  %d  %d\n",team_id,player_id,energy);
        memset(result, '\0', sizeof(result));
        snprintf(result, sizeof(result), "%d", energy);
        send_message(ENERGY_ID, result);
    }
    alarm(5);
}

void busy_wait(int ms) {
    clock_t start = clock();
    while ((clock() - start) * 1000 / CLOCKS_PER_SEC < ms);
}

void update_energy(int time){
    double energy_damping_factor = 3 * ((double)time / ((double)config.max_time * 1000.0));
    int new_energy = (int) (config.max_energy_per_round * energy_damping_factor);
    energy = (energy-new_energy >= 0) ? (energy-new_energy) : 0;
}