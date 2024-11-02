// team.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "team.h"
#include "player.h"
#include "config.h"

int team_id;
Config *team_config;

void handle_team_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Team %d: Received referee's signal, starting jump\n", team_id + 1);
    }
}

void team_main(int id, Config *config) {
    team_id = id;
    team_config = config;

    signal(SIGUSR1, handle_team_signal);

    // Create player processes
    pid_t players[3];
    for (int i = 0; i < 3; i++) {
        if ((players[i] = fork()) == 0) {
            player_main(id, i, config);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for referee signal indefinitely
    while (1) pause();
}
