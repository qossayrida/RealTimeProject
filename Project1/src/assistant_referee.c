#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "player.h"
#include "config.h"

Team team_data;
pid_t players[3];
int pipe_player[2];  // Pipe to communicate with players
char fifo_name[2][50];
int fifo_to_highest,fifo_to_lowest;
struct timespec start_time, end_time;
char buffer[3][50];


void handle_start_signal(int signum);
void handle_term_signal();
void go_to_jump();
void go_to_pull();

void assistant_referee_main(int id, Config *config, int *pipe_fd) {
    team_data.team_id = id;
    team_data.turn = 0;

    signal(SIGUSR1, handle_start_signal);
    signal(SIGTERM, handle_term_signal);


    // Create a pipe for communication with the player processes
    if (pipe(pipe_player) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Create a FIFO (named pipe) for inter-player communication

    sprintf(fifo_name[0], "/tmp/team%d_fifo_for_highest_player",  team_data.team_id + 1);
    if ( mknod(fifo_name[0], S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name[0]);
        exit(1);
    }

    sprintf(fifo_name[1], "/tmp/team%d_fifo_for_lowest_player",  team_data.team_id + 1);
    if ( mknod(fifo_name[1], S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name[1]);
        exit(1);
    }


    // Create player processes
    for (int i = 0; i < 3; i++) {
        if ((players[i] = fork()) == 0) {
//            char name [50];
//            snprintf(name, sizeof(name), "bungee_team%d_player%d", id+1,i+1);

            fifo_to_highest = open(fifo_name[0], O_RDWR);
            fifo_to_lowest = open(fifo_name[1], O_RDWR);
            player_main(id, i, config, pipe_player, fifo_to_highest,fifo_to_lowest);  // Pass read end and FIFO
            exit(EXIT_SUCCESS);
        }
        team_data.players[i] = players[i];
    }

    // Send team data to the main referee
    if (write(pipe_fd[1], &team_data, sizeof(Team)) == -1) {
        perror("Failed to write team data to referee");
    }

    // Wait for signals indefinitely
    pause();

    while (1){
        if(read(pipe_player[0], buffer[0], sizeof(buffer[0])) == -1){
            perror("Failed to read from player");
            exit(EXIT_FAILURE);
        }

        if (strncmp(buffer[0], "JUMP_END",8) == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            double jump_interval = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
            go_to_pull();


            snprintf(buffer[1], sizeof(buffer[1]), " %d: JT %.2f",team_data.team_id+1, jump_interval);
            if (write(pipe_fd[1], buffer[1], strlen(buffer[1])) == -1) {
                perror("Failed to write team data to referee");
            }

        }else if (strncmp(buffer[0], "PULL_END",8) == 0) {

            go_to_jump();


        }else if (strncmp(buffer[0], "ENERGY",6) == 0) {
            // ENERGY_OUT 0
            char *token = strtok(buffer[0], "#");
            token = strtok(NULL, "#");
            int player_id = atoi(token);
            token = strtok(NULL, "#");
            int energy = atoi(token);
            team_data.players[player_id] = -1;

            snprintf(buffer[2], sizeof(buffer[2]), " %d: EO %d",team_data.team_id+1, player_id+1);
            if (write(pipe_fd[1], buffer[2], strlen(buffer[2])) == -1) {
                perror("Failed to write team data to referee");
            }
        }
    }

}

void handle_start_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Assistant Referee for Team %d: Received signal from referee\n", team_data.team_id + 1);
        go_to_jump();
    }
}

void go_to_jump(){
    // Send signal to the player with the current turn to allow them to jump

    // p = team_data.players[team_data.turn] == -1
    // a = team_data.players[(team_data.turn+1) %3] == -1
    // q = team_data.players[(team_data.turn+2) %3] == -1
    // (p && (a || q)) || (a && q)

    if (( team_data.players[team_data.turn] == -1 && (team_data.players[(team_data.turn+1) %3] == -1  || team_data.players[(team_data.turn+2) %3] == -1))
    || (team_data.players[(team_data.turn+1) %3] == -1  && team_data.players[(team_data.turn+2) %3] == -1)) {
        exit(EXIT_SUCCESS);
    }else if( team_data.players[team_data.turn] == -1){
        team_data.turn = (team_data.turn + 1) % 3;
    }
    printf("Team %d: Sending signal to Player %d to jump\n", team_data.team_id + 1, team_data.turn + 1);

    pid_t current_player = players[team_data.turn];

    // Record the start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);


    kill(current_player, SIGUSR1);

    // Move to the next player's turn
    team_data.turn = (team_data.turn + 1) % 3;
}


void go_to_pull(){
    if(team_data.players[team_data.turn] != -1){
        pid_t current_player = players[team_data.turn];
        kill(current_player, SIGUSR2);
    }
    if(team_data.players[(team_data.turn+1) %3] != -1){
        pid_t second_player = players[(team_data.turn+1) %3];
        kill(second_player, SIGUSR2);
    }
    if (team_data.players[team_data.turn] == -1 && team_data.players[(team_data.turn+1) %3] == -1)
        exit(EXIT_SUCCESS);
}

void handle_term_signal() {
    close(fifo_to_highest);
    if (unlink(fifo_name[0]) == 0) {
        printf("fifo: fifo%d has been removed\n", getpid());
    }
    close(fifo_to_lowest);
    if (unlink(fifo_name[1]) == 0) {
        printf("fifo: fifo%d has been removed\n", getpid());
    }

    // Reset SIGTERM to default and re-raise it
    signal(SIGTERM, SIG_DFL);
    raise(SIGTERM);
}

