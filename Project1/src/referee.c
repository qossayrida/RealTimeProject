// referee.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "referee.h"
#include "team.h"
#include "config.h"

void referee_main(int *referee_pipe, Config *config) {
    close(referee_pipe[0]); // Close reading end in referee

    pid_t teams[3];
    for (int i = 0; i < 3; i++) {
        if ((teams[i] = fork()) == 0) {
            // Each child process represents a team
            team_main(i, config);
            exit(EXIT_SUCCESS);
        }
    }

    // Coordinate turns
    while (1) {
        for (int i = 0; i < 3; i++) {
            printf("Referee: Team %d's turn\n", i + 1);
            kill(teams[i], SIGUSR1); // Signal the team to start

            // Wait for result through pipe
            char buffer[256];
            read(referee_pipe[1], buffer, sizeof(buffer));
            printf("Referee received: %s\n", buffer);

            // Pause between teams
            sleep(1);
        }
    }
}
