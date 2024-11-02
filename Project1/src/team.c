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
//just 

void handle_team_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Team %d: Received referee's signal, starting jump\n", team_id + 1);
    }


}

void team_main(int team_id, Config *config, int *pipe) {


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



//                                main (pipe r dis w dis)
//
//                               referee (pipe r dis w dis)  *pipe r dis w dis*
//
//       team1 *pipe r dis w dis*    team2 *pipe r dis w dis*   team3 *pipe r dis w dis*
//
//       player1 player2 player3    player1 player2 player3    player1 player2 player3
