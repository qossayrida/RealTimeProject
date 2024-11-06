// team.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "player.h"
#include "config.h"

int team_id;
Config *team_config;
pid_t players[3];

void handle_team_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Team %d: Received referee's signal, starting jump\n", team_id + 1);
    }
}

void handle_segterm(int signum) {
    printf("Team %d: Terminating\n", team_id + 1);
    for (int i = 0; i < 3; i++) {
        kill(players[i], SIGTERM);
    }
    exit(EXIT_SUCCESS);
}

void assistant_referee_main(int id, Config *config, int *pipe) {


    team_id = id;
    team_config = config;

    signal(SIGTERM, handle_segterm);
    signal(SIGUSR1, handle_team_signal);

    // Create player processes

    for (int i = 0; i < 3; i++) {
        if ((players[i] = fork()) == 0) {
            player_main(id, i, config);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for referee signal indefinitely
    while (1) pause();
}

