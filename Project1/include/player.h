// include/player.h
#ifndef PLAYER_H
#define PLAYER_H

// Function for a player to perform a jump and stabilize
void player_jump(int team_id, int player_id, int min_time, int max_time);

// Function for two players to pull up a teammate after the jump
void player_pull(int team_id, int player1, int player2, int min_time, int max_time);

#endif // PLAYER_H
 
