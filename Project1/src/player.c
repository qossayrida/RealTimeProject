#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "player.h"
#include "config.h"

void player_main(int team_id, int player_id, Config *config) {
    srand(time(NULL) ^ (getpid() << 16));

    int energy = config->initial_energy_team_a; // Adjust based on team
    int stabilization_time, pull_time;

    while (1) {
        stabilization_time = (rand() % (config->stabilization_time_max - config->stabilization_time_min)) 
                                + config->stabilization_time_min;
        printf("Team %d: Player %d jumping... Stabilization time: %d\n", team_id + 1, player_id + 1, stabilization_time);
        usleep(stabilization_time * 1000);

        // Signal other players for pull
        pull_time = (rand() % (config->pull_time_max - config->pull_time_min)) + config->pull_time_min;
        printf("Team %d: Pulling player %d... Pull time: %d\n", team_id + 1, player_id + 1, pull_time);
        usleep(pull_time * 1000);

        // Decrease energy for jump and pull
        energy -= 10;
        if (energy <= 0) {
            printf("Team %d: Player %d is out of energy\n", team_id + 1, player_id + 1);
            exit(0);
        }
    }
}
