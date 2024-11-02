 
// src/player.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "player.h"

void player_jump(int team_id, int player_id, int min_time, int max_time) {
    int time_to_stabilize = min_time + rand() % (max_time - min_time + 1);
    printf("Team %d: Player %d jumping... Stabilization time: %d\n", team_id, player_id, time_to_stabilize);
    
    sleep(time_to_stabilize); // Simulate stabilization time
    printf("Team %d: Player %d stabilized.\n", team_id, player_id);
}

void player_pull(int team_id, int player1, int player2, int min_time, int max_time) {
    int pull_time = min_time + rand() % (max_time - min_time + 1);
    printf("Team %d: Players %d and %d pulling... Pull time: %d\n", team_id, player1, player2, pull_time);
    
    sleep(pull_time); // Simulate pull time
    printf("Team %d: Player pulled up successfully.\n", team_id);
}
