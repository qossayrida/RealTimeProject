#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>
#include "config.h"

Team teams[3];
pid_t assistant_referees[3];
Config game_config;

void handle_alarm(int signum);
void end_game();

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

    char fifo_name_str[50];
    sprintf(fifo_name_str, "/tmp/bungee_public");
    if ( mknod(fifo_name_str, S_IFIFO | 0666, 0) < 0 ) {
        perror(fifo_name_str);
        exit(1);
    }

    // Create assistant referee processes
    for (int i = 0; i < 3; i++) {
        if ((assistant_referees[i] = fork()) == 0) {
            char name [50];
            char id_str[10];
            snprintf(name, sizeof(name), "bungee_assistant_referee_%d", i+1);
            snprintf(id_str, sizeof(id_str), "%d", i);
            execlp("./bin/assistant_referee", name, id_str,fifo_name_str, argv[1], NULL);
            perror("execlp failed for assistant_referee");
            exit(EXIT_FAILURE);
        }
    }

    int fifo_fd = open(fifo_name_str, O_RDONLY);

    // Receive team data from each assistant referee
    for (int i = 0; i < 3; i++){
        Team team_data;
        if (read(fifo_fd, &team_data, sizeof(Team)) == -1) {
            perror("Failed to read from assistant referee");
            return EXIT_FAILURE;
        }
        teams[team_data.team_id] = team_data;
    }


    // Send signal to assistant referees to start the game
    for (int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("Team %d --> Player %d has initial energy %d\n", i + 1, j + 1 , teams[i].initial_energy[j]);
        }
        kill(assistant_referees[i], SIGUSR1);
        printf("Referee: Sent start signal to Team %d\n", i + 1);
    }

    Message message_buffer[10];

    while (1){

        int bytes_read = read(fifo_fd, message_buffer, sizeof(message_buffer));

        if(bytes_read == -1){
            perror("Failed to read from player");
            exit(EXIT_FAILURE);
        }

        int num_messages = bytes_read / sizeof(Message);

        for(int i = 0; i < num_messages; i++) {

            if (message_buffer[i].type == JUMP_START_ID) {
                printf("Team %d --> Player %d is starting to jump\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
            } else if (message_buffer[i].type == PULL_START_ID) {
                printf("Team %d --> Player %d is starting to pull\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
            } else if (message_buffer[i].type == JUMP_END_ID) {
                printf("Team %d --> Player %d has finished jumping\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1);
            } else if (message_buffer[i].type == PULL_END_ID) {
                teams[message_buffer[i].team_id].score += (int) (0.7 * (game_config.max_score * (game_config.stabilization_time_min/1000.0) / atof(message_buffer[i].content)));
                printf("Team %d --> Player %d has finished pulling with time %s with score %d\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1,message_buffer[i].content,teams[message_buffer[i].team_id].score);
                if (teams[message_buffer[i].team_id].score >= game_config.max_score) {
                    end_game();
                }
            } else if (message_buffer[i].type == ENERGY_ID) {
                printf("Team %d --> Player %d has %s energy\n", message_buffer[i].team_id + 1, message_buffer[i].player_id + 1, message_buffer[i].content);
            } else if (message_buffer[i].type == TEAM_STOP) {
                printf("Team %d has stopped\n", message_buffer[i].team_id + 1);
            }
        }

    }

    return EXIT_SUCCESS;
}

void handle_alarm(int signum) {
    printf("Referee: Maximum time reached. Ending game.\n");
    end_game();
}

void end_game(){

    int max_score = teams[0].score;
    int winning_team_id = 0;
    int is_draw = 1;

    for (int j = 1; j < 3; j++) {
        if (teams[j].score > max_score) {
            max_score = teams[j].score;
            winning_team_id = j;
            is_draw = 0;  // There’s a unique highest score, so it’s not a draw
        } else if (teams[j].score < max_score) {
            is_draw = 0;  // Scores are not all equal
        }
    }

    if (is_draw) {
        printf("Referee: The game is a draw\n");
    } else {
        printf("Referee: Team %d has won the game with a score of %d\n", winning_team_id + 1, max_score);
    }

    system("rm /tmp/bungee_*");
    printf("Referee: Removed all FIFOs\n");

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kill(teams[i].players[j], SIGTERM);
        }
        kill(assistant_referees[i], SIGTERM);
    }

    exit(EXIT_SUCCESS);
}