#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "player.h"
#include "config.h"

int team_id;
pid_t players[3];

void handle_team_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Team %d: Received referee's signal, starting jump\n", team_id + 1);
    }
}

void assistant_referee_main(int id, Config *config, int *pipe_fd) {

    team_id = id;
    Team team_data;
    team_data.team_id = id;
    team_data.score = 0;
    team_data.turn = 0;

    signal(SIGUSR1, handle_team_signal);

    // Create player processes
    for (int i = 0; i < 3; i++) {
        if ((players[i] = fork()) == 0) {
            player_main(id, i, config);
            exit(EXIT_SUCCESS);
        }
        team_data.players[i] = players[i];
    }

    // Send team data to the main referee
    if (write(pipe_fd[1], &team_data, sizeof(Team)) == -1) {
        perror("Failed to write team data to referee");
    }

    while(1)
        pause();
}
