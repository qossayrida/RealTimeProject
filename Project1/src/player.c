#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "player.h"
#include "config.h"

int global_team_id;
int *global_pipe_read_fd;
int is_turn = 0;  // Flag to check if the player is allowed to jump
int energy;

void handle_player_signal(int signum);
void jump(int team_id, int player_id, Config *config, int fifo_fd);

void player_main(int team_id, int player_id, Config *config, int *pipe_read_fd, int fifo_fd) {
    global_team_id = team_id;
    global_pipe_read_fd = pipe_read_fd;
    srand(time(NULL) ^ (getpid() << 16));

    energy = config->initial_energy_team_a;  // Adjust based on team

    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, handle_player_signal);

    while (1) {
        // Wait until it’s this player’s turn (indicated by `is_turn` flag)
        while (!is_turn) {
            pause();  // Wait for the signal to start the jump
        }

        if (is_turn == 1) {
            jump(team_id, player_id, config, fifo_fd);
        } else if (is_turn == 2) {
            printf("Team %d: Player %d pulling...\n", team_id + 1, player_id + 1);
        }

        is_turn = 0;  // Reset turn flag after receiving signal
    }
}

void handle_player_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Team %d: Received signal to jump from assistant referee\n", global_team_id + 1);
        is_turn = 1;  // Set the turn flag to allow the player to jump
    } else if (signum == SIGUSR2) {
        printf("Team %d: Received signal to pull from another player\n", global_team_id + 1);
        is_turn = 2;  // Set the turn flag to allow the player to pull
    }
}

void jump(int team_id, int player_id, Config *config, int fifo_fd) {
    int stabilization_time, pull_time;

    stabilization_time = (rand() % (config->stabilization_time_max - config->stabilization_time_min))
                         + config->stabilization_time_min;
    printf("Team %d: Player %d jumping... Stabilization time: %d\n", team_id + 1, player_id + 1, stabilization_time);
    usleep(stabilization_time * 1000);

    // Decrease energy for jump and pull
    energy -= 10;
    if (energy <= 0) {
        printf("Team %d: Player %d is out of energy\n", team_id + 1, player_id + 1);
        exit(0);
    }

    char result[50];
    snprintf(result, sizeof(result), "JUMP_END");
    if (write(global_pipe_read_fd[1], result, strlen(result)) == -1) {
        perror("Failed to write team data to referee");
    }
}
