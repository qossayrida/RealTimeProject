#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "player.h"
#include "config.h"

Team team_data;
pid_t players[3];
int pipe_player[2];  // Pipe to communicate with players
char fifo_name[20];
int fifo_fd;

void handle_team_signal(int signum);
void handle_term_signal();

void assistant_referee_main(int id, Config *config, int *pipe_fd) {
    team_data.team_id = id;
    team_data.score = 0;
    team_data.turn = 0;

    signal(SIGUSR1, handle_team_signal);
    signal(SIGTERM, handle_term_signal);


    // Create a pipe for communication with the player processes
    if (pipe(pipe_player) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Create a FIFO (named pipe) for inter-player communication

    sprintf(fifo_name, "/tmp/team%d_fifo",  team_data.team_id);
    if ( mknod(fifo_name, S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name);
        exit(1);
    }


    // Create player processes
    for (int i = 0; i < 3; i++) {
        if ((players[i] = fork()) == 0) {
            fifo_fd = open(fifo_name, O_RDWR);
            player_main(id, i, config, pipe_player, fifo_fd);  // Pass read end and FIFO
            exit(EXIT_SUCCESS);
        }
        team_data.players[i] = players[i];
    }

    // Send team data to the main referee
    if (write(pipe_fd[1], &team_data, sizeof(Team)) == -1) {
        perror("Failed to write team data to referee");
    }

    // Wait for signals indefinitely
    pause();

    while (1){
        char buffer[256];
        if(read(pipe_player[0], buffer, sizeof(buffer)) == -1){
            perror("Failed to read from player");
            exit(EXIT_FAILURE);
        }
        printf("Assistant referee: Received message from player %s\n", buffer);
    }

}

void handle_team_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Assistant Referee for Team %d: Received signal from referee\n", team_data.team_id + 1);

        // Send signal to the player with the current turn to allow them to jump
        pid_t current_player = players[team_data.turn];
        kill(current_player, SIGUSR1);

        // Move to the next player's turn
        team_data.turn = (team_data.turn + 1) % 3;
    }
}


void handle_term_signal() {
    close(fifo_fd);
    if ( (unlink(fifo_name)) == 0 )
        printf("fifo: fifo%d has been removed\n", getpid());
}