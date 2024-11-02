 
// include/team.h
#ifndef TEAM_H
#define TEAM_H

#include "config.h"

// Team IDs
#define TEAM_A 1
#define TEAM_B 2
#define TEAM_C 3

// Function for performing a team action
void team_action(int team_id, const Config *config);

// Function to calculate score inversely proportional to time taken
int calculate_score(const Config *config);

#endif // TEAM_H
