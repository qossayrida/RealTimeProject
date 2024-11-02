#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "config.h"
#include "referee.h"
#include "team.h"
#include "player.h"

// Game configuration
Config game_config;
int referee_pipe[2]; // Pipe for referee-main communication

// Signal handler for referee communication
void handle_referee_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Referee: Signal received to start game\n");
    }
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

    // Set up pipe for referee communication
    if (pipe(referee_pipe) == -1) {
        perror("Pipe creation failed");
        return EXIT_FAILURE;
    }

    // Fork process to start referee and game logic
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: referee's game control
        referee_main(referee_pipe, &game_config);
        exit(EXIT_SUCCESS);
    }

    // Parent process (main) waits for signals
    signal(SIGUSR1, handle_referee_signal);

    // Wait for referee's signal and instructions
    close(referee_pipe[1]); // Close writing end in the main process

    while (1) {
        char buffer[256];
        if (read(referee_pipe[0], buffer, sizeof(buffer)) > 0) {
            printf("Main received: %s\n", buffer);
        }
    }

    return EXIT_SUCCESS;
}
