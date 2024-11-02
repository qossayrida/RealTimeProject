// src/referee.c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "config.h"
#include "team.h"
#include "referee.h"

// Game state and signal handling
volatile sig_atomic_t team_ready = 0;

// Signal handler for team readiness
void team_ready_signal(int signum) {
    team_ready = 1;
}

void referee_main(const Config *config) {
    int score_a = 0, score_b = 0, score_c = 0;
    
    // Set up signal handling for team readiness
    signal(SIGUSR1, team_ready_signal);

    while (1) {
        team_ready = 0;
        printf("Referee: Team A's turn\n");
        team_action(TEAM_A, config);

        // Wait for team to signal readiness
        while (!team_ready);
        
        // Calculate and update score
        score_a += calculate_score(config);

        // Check if any team has reached the max score
        if (score_a >= config->max_score) {
            printf("Team A wins with a score of %d!\n", score_a);
            break;
        }

        // Repeat for teams B and C
        team_ready = 0;
        printf("Referee: Team B's turn\n");
        team_action(TEAM_B, config);

        while (!team_ready);
        score_b += calculate_score(config);

        if (score_b >= config->max_score) {
            printf("Team B wins with a score of %d!\n", score_b);
            break;
        }

        team_ready = 0;
        printf("Referee: Team C's turn\n");
        team_action(TEAM_C, config);

        while (!team_ready);
        score_c += calculate_score(config);

        if (score_c >= config->max_score) {
            printf("Team C wins with a score of %d!\n", score_c);
            break;
        }
    }
}
 
