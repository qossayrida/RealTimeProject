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
#include <sys/prctl.h>

Team teams[3];
pid_t assistant_referees[3];
Config game_config;

void handle_alarm(int signum);

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

    // Set up alarm for maximum time
    signal(SIGALRM, handle_alarm);
    printf("Referee: Maximum time set to %d seconds\n", game_config.max_time);
    alarm(game_config.max_time);

    // Create a single pipe for communication with all assistant referees
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        return EXIT_FAILURE;
    }

    // Create assistant referee processes
    for (int i = 0; i < 3; i++) {
        if ((assistant_referees[i] = fork()) == 0) {
           char name [50];
            snprintf(name, sizeof(name), "bungee_assistant_referee_%d", i+1);
            prctl(PR_SET_NAME, (unsigned long) name, 0, 0, 0);

            assistant_referee_main(i, &game_config, pipe_fd);
            exit(EXIT_SUCCESS);
        }
    }
    close(pipe_fd[1]);  // Close write end in parent

    // Receive team data from each assistant referee
    for (int i = 0; i < 3; i++){
        Team team_data;
        if (read(pipe_fd[0], &team_data, sizeof(Team)) == -1) {
            perror("Failed to read from assistant referee");
            return EXIT_FAILURE;
        }
        teams[team_data.team_id] = team_data;
    }


    // Send signal to assistant referees to start the game
    for (int i = 0; i < 3; i++) {
        kill(assistant_referees[i], SIGUSR1);
        printf("Referee: Sent start signal to Team %d\n", i + 1);
    }


    while (1){
        char buffer[50];
        memset(buffer, '\0', sizeof(buffer));
        if(read(pipe_fd[0], buffer, sizeof(buffer)) == -1){
            perror("Failed to read from assistant referee");
            return EXIT_FAILURE;
        }
        //printf("Referee: Received message from Team  %s\n", buffer);
    }

    return EXIT_SUCCESS;
}

void handle_alarm(int signum) {
    printf("Referee: Maximum time reached. Ending game.\n");

    system("rm /tmp/team*_fifo*");
    printf("Referee: Removed all FIFOs\n");

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kill(teams[i].players[j], SIGKILL);
        }
        kill(assistant_referees[i], SIGKILL);
    }

    exit(EXIT_SUCCESS);
}
