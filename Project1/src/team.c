// src/team.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "team.h"
#include "player.h"
#include "config.h"

// Function for team action (jump + pull)
void team_action(int team_id, const Config *config) {
    printf("Team %d: Player 1 is jumping!\n", team_id);
    
    // First player jumps, others wait
    player_jump(team_id, 1, config->stabilization_time_min, config->stabilization_time_max);
    
    printf("Team %d: Pulling player 1 up!\n", team_id);
    player_pull(team_id, 2, 3, config->pull_time_min, config->pull_time_max);

    // Signal readiness to referee
    kill(getppid(), SIGUSR1);
}

// Calculate score inversely proportional to time taken
int calculate_score(const Config *config) {
    // Placeholder for score calculation based on time
    int score = 10;  // Example static score
    printf("Score for this round: %d\n", score);
    return score;
}
 
