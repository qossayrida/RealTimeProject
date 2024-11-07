#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "player.h"
#include "config.h"

int global_team_id;
int is_turn = 0;  // Flag to check if the player is allowed to jump

void handle_player_signal(int signum);

void player_main(int team_id, int player_id, Config *config) {
    global_team_id = team_id;
    srand(time(NULL) ^ (getpid() << 16));

    int energy = config->initial_energy_team_a; // Adjust based on team
    int stabilization_time, pull_time;

    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, handle_player_signal);

    while (1) {
        // Wait until it’s this player’s turn (indicated by `is_turn` flag)
        while (!is_turn) {
            pause();  // Wait for the signal to start the jump
        }
        is_turn = 0;  // Reset turn flag after receiving signal

        // Jump process
        stabilization_time = (rand() % (config->stabilization_time_max - config->stabilization_time_min))
                             + config->stabilization_time_min;
        printf("Team %d: Player %d jumping... Stabilization time: %d\n", team_id + 1, player_id + 1, stabilization_time);
        usleep(stabilization_time * 1000);

        // Signal other players for pull
        pull_time = (rand() % (config->pull_time_max - config->pull_time_min)) + config->pull_time_min;
        printf("Team %d: Player %d pulling... Pull time: %d\n", team_id + 1, player_id + 1, pull_time);
        usleep(pull_time * 1000);

        // Decrease energy for jump and pull
        energy -= 10;
        if (energy <= 0) {
            printf("Team %d: Player %d is out of energy\n", team_id + 1, player_id + 1);
            exit(0);
        }
    }
}

void handle_player_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Team %d: Received signal to jump from assistant referee\n", global_team_id + 1);
        is_turn = 1;  // Set the turn flag to allow the player to jump
    }
}
