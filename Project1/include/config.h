#ifndef CONFIG_H
#define CONFIG_H

#define READY_TO_PLAY 0
#define JUMP_START_ID 1
#define PULL_START_ID 2
#define JUMP_END_ID 3
#define PULL_END_ID 4
#define ENERGY_ID 5
#define TEAM_STOP 6

typedef struct {
    int max_score;
    int max_time;
    int initial_energy_max[3];
    int initial_energy_min[3];
    int stabilization_time_min;
    int stabilization_time_max;
    int rest_time_max;
    int rest_time_min;
    int pull_time_min;
    int pull_time_max;
} Config;

typedef struct {
    int team_id;
    int score;
    int turn;
    int initial_energy[3];
    int deceased [3];
    pid_t players[3];
} Team;


typedef struct {
    int type;
    int player_id;
    int team_id;
    char content[50];
} Message;


int load_config(const char *filename, Config *config);

#endif // CONFIG_H
