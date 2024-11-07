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
Team team_data;

void handle_team_signal(int signum);

void assistant_referee_main(int id, Config *config, int *pipe_fd) {

    team_id = id;
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


void handle_team_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Assistant Referee for Team %d: Received signal from referee\n", team_id + 1);

        // Send signal to the player with the current turn to allow them to jump
        pid_t current_player = players[team_data.turn];
        kill(current_player, SIGUSR1);

        // Move to the next player's turn
        team_data.turn = (team_data.turn + 1) % 3;
    }
}