// include/player.h

#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"

// Function to initialize and handle each player's actions
void player_main(int team_id, int player_id, Config *config, int *pipe_read_fd, int fifo_to_highest, int fifo_to_lowest);


#endif // PLAYER_H
