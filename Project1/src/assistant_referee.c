#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"

Team team_data;
pid_t players[3];
char fifo_name[3][50];
int fifo_to_highest,fifo_to_lowest;
struct timespec start_time, end_time;
char buffer[3][50];
int fifo_to_referee,is_jump = -1,is_pull = -1;
double jump_interval;
void handle_start_signal(int signum);
void go_to_jump();
void go_to_pull();
void team_stop();
void countine_with_two_player(int dead_player);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <id> <fifo_name> <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }


    team_data.team_id = atoi(argv[1]);
    fifo_to_referee = open(argv[2], O_WRONLY);

    team_data.turn = 0;
    memset(team_data.deceased, 0, sizeof(team_data.deceased));
    signal(SIGUSR1, handle_start_signal);


    // Create a FIFO (named pipe) for inter-player communication
    sprintf(fifo_name[0], "/tmp/bungee_team%d_fifo_for_highest",  team_data.team_id + 1);
    if ( mknod(fifo_name[0], S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name[0]);
        exit(1);
    }

    sprintf(fifo_name[1], "/tmp/bungee_team%d_for_lowest",  team_data.team_id + 1);
    if ( mknod(fifo_name[1], S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name[1]);
        exit(1);
    }

    sprintf(fifo_name[2], "/tmp/bungee_team%d",  team_data.team_id + 1);
    if ( mknod(fifo_name[2], S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name[2]);
        exit(1);
    }

    // Create player processes
    for (int i = 0; i < 3; i++) {
        if ((players[i] = fork()) == 0) {
            char name [50];
            snprintf(name, sizeof(name), "bungee_team%d_player%d", team_data.team_id+1,i+1);


            char id_player_str[10],id_team_str[10];
            snprintf(id_player_str, sizeof(id_player_str), "%d", i);
            snprintf(id_team_str, sizeof(id_team_str), "%d", team_data.team_id);


            // Pass both the pipe file descriptors and FIFO names
            execlp("./bin/player", name, id_player_str,id_team_str,fifo_name[0], fifo_name[1],fifo_name[2],argv[3], NULL);
            perror("execlp failed for player");
            exit(EXIT_FAILURE);
        }
        team_data.players[i] = players[i];
    }


    int fifo_fd = open(fifo_name[2], O_RDONLY);

    for (int i = 0; i < 3; i++) {
        Message msg;
        if (read(fifo_fd, &msg, sizeof(Message)) > 0) {
            if (msg.type == READY_TO_PLAY) {
                team_data.initial_energy[msg.player_id] = atoi(msg.content);
            }
        }
    }

    // Send team data to the main referee
    if (write(fifo_to_referee, &team_data, sizeof(Team)) == -1) {
        perror("Failed to write team data to referee");
    }

    // Wait for signals indefinitely
    pause();



    Message message_buffer[10];

    while (1){

        int bytes_read = read(fifo_fd, message_buffer, sizeof(message_buffer));

        if(bytes_read == -1){
            perror("Failed to read from player");
            exit(EXIT_FAILURE);
        }

        int num_messages = bytes_read / sizeof(Message);
        for(int i = 0; i < num_messages; i++) {

            if (message_buffer[i].type == PULL_START_ID){
                is_pull = message_buffer[i].player_id;
                if(write(fifo_to_referee, &message_buffer[i], sizeof(message_buffer[i]))== -1)
                    perror("Failed to write team data to referee");

            }else if (message_buffer[i].type == JUMP_START_ID) {
                is_jump = message_buffer[i].player_id;
                if(write(fifo_to_referee, &message_buffer[i], sizeof(message_buffer[i]))== -1)
                    perror("Failed to write team data to referee");

            } else if (message_buffer[i].type == JUMP_END_ID) {
                is_jump=-1;
                clock_gettime(CLOCK_MONOTONIC, &end_time);
                jump_interval = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

                go_to_pull();

                memset(message_buffer[i].content, '\0', sizeof(message_buffer[i].content));
                snprintf(message_buffer[i].content, sizeof(message_buffer[i].content), "%.2f",jump_interval);
                if(write(fifo_to_referee, &message_buffer[i], sizeof(message_buffer[i]))== -1)
                    perror("Failed to write team data to referee");


                if(team_data.deceased[message_buffer[i].player_id] == 1)
                    countine_with_two_player(message_buffer[i].player_id);

            } else if (message_buffer[i].type == PULL_END_ID) {
                is_pull=-1;
                go_to_jump();
                memset(message_buffer[i].content, '\0', sizeof(message_buffer[i].content));
                snprintf(message_buffer[i].content, sizeof(message_buffer[i].content), "%.2f",jump_interval);
                if(write(fifo_to_referee, &message_buffer[i], sizeof(message_buffer[i]))== -1)
                    perror("Failed to write team data to referee");

                if(team_data.deceased[message_buffer[i].player_id] == 1)
                    countine_with_two_player(message_buffer[i].player_id);

            } else if (message_buffer[i].type == ENERGY_ID) {
                int energy = atoi(message_buffer[i].content);

                if(write(fifo_to_referee, &message_buffer[i], sizeof(message_buffer[i]))== -1)
                    perror("Failed to write team data to referee");

                if (energy == 0) {
                    team_data.deceased[message_buffer[i].player_id] = 1;


                    if ((team_data.deceased[0] && team_data.deceased[1]) ||
                        (team_data.deceased[1] && team_data.deceased[2]) ||
                        (team_data.deceased[0] && team_data.deceased[2]))
                        team_stop();

                    if ((is_jump != message_buffer[i].player_id) && (is_pull != message_buffer[i].player_id ))
                        countine_with_two_player(message_buffer[i].player_id);
                }
            }
        }    }

}

void go_to_jump(){
    if(team_data.deceased[team_data.turn] == 1){
        printf("Team %d: Player %d is deceased. Skipping turn to player %d\n", team_data.team_id + 1, team_data.turn, (team_data.turn + 1) % 3);
        team_data.turn = (team_data.turn + 1) % 3;
    }


    pid_t current_player = players[team_data.turn];

    // Record the start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);


    kill(current_player, SIGUSR1);

    // Move to the next player's turn
    team_data.turn = (team_data.turn + 1) % 3;
}


void go_to_pull(){
    if(!team_data.deceased[team_data.turn]){
        pid_t current_player = players[team_data.turn];
        kill(current_player, SIGUSR2);
    }
    if(!team_data.deceased[(team_data.turn+1) %3]){
        pid_t second_player = players[(team_data.turn+1) %3];
        kill(second_player, SIGUSR2);
    }
}

void countine_with_two_player(int dead_player){
    kill(team_data.players[dead_player], SIGTERM);
    kill(team_data.players[(dead_player+1)%3], SIGQUIT);
    kill(team_data.players[(dead_player+2)%3], SIGQUIT);
}

void team_stop(){
    for (int i = 0; i < 3; i++)
        if (team_data.deceased[i] != 1)
            kill(team_data.players[i], SIGTERM);

    exit(EXIT_SUCCESS);
}


void handle_start_signal(int signum) {
    if (signum == SIGUSR1)
        go_to_jump();
}
