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

    int team_pipes[2];  // Pipes for each team
    pid_t teams[3];

    // Create a pipe for each team
    if (pipe(team_pipes) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }


    // Fork each team process
    for (int i = 0; i < 3; i++) {
        if ((teams[i] = fork()) == 0) {
            // Each child process represents a team
            team_main(i, config, team_pipes);  // Pass the pipe for the team
            exit(EXIT_SUCCESS);
        }
    }

    time_t start_time = time(NULL);


    while (1) {
        time_t current_time = time(NULL);
        int elapsed_time = (int)(current_time - start_time);

        // Check if maximum time is reached
        if (elapsed_time >= config->max_time) {
            printf("Referee: Maximum time reached. Ending game.\n");
            break;
        }

        for (int i = 0; i < 3; i++) {
            printf("Referee: Team %d's turn\n", i + 1);
            kill(teams[i], SIGUSR1); // Signal the team to start

            // Wait for result through the team's pipe
            char buffer[256];
            read(team_pipes[i][0], buffer, sizeof(buffer));
            printf("Referee received from Team %d: %s\n", i + 1, buffer);
        }
    }

}
