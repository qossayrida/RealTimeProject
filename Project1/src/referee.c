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

    int team_pipes[3][2];  // Pipes for each team
    pid_t teams[3];

    // Create a pipe for each team
    for (int i = 0; i < 3; i++) {
        if (pipe(team_pipes[i]) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Fork each team process
    for (int i = 0; i < 3; i++) {
        if ((teams[i] = fork()) == 0) {
            // Each child process represents a team
            team_main(i, config, team_pipes[i]);  // Pass the pipe for the team
            exit(EXIT_SUCCESS);
        }
    }

    time_t start_time = time(NULL);


}
