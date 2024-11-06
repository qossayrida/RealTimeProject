// main_referee.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>
#include "config.h"
#include "assistant_referee.h"
#include "player.h"

pid_t teams[3];
Config game_config;

void handle_alarm(int signum) {
    printf("Referee: Maximum time reached. Ending game.\n");
    // Kill all child processes
    for (int i = 0; i < 3; i++) {
        kill(teams[i], SIGTERM);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load configuration
    if (load_config(argv[1], &game_config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }


    // Referee setup
    signal(SIGALRM, handle_alarm);
    alarm(game_config.max_time);

    int team_pipes[2];  // Pipes for each team

    // Create a pipe for each team
    if (pipe(team_pipes) == -1) {
        perror("Pipe creation failed");
        return EXIT_FAILURE;
    }

    // Fork each team process
    for (int i = 0; i < 3; i++) {
        if ((teams[i] = fork()) == 0) {
            // Each child process represents a team
            assistant_referee_main(i, &game_config, team_pipes);  // Pass the pipe for the team
            exit(EXIT_SUCCESS);
        }
    }

    // Game loop
    while (1) {
        for (int i = 0; i < 3; i++) {
            printf("Referee: Team %d's turn\n", i + 1);
            kill(teams[i], SIGUSR1); // Signal the team to start

            // Wait for result through the team's pipe
            char buffer[256];
            read(team_pipes[0], buffer, sizeof(buffer));
            printf("Referee received from Team %d: %s\n", i + 1, buffer);
        }
    }

    return EXIT_SUCCESS;
}

