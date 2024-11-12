#ifndef CONFIG_H
#define CONFIG_H

enum {
    READY_TO_PLAY,
    JUMP_START_ID,
    PULL_START_ID,
    JUMP_END_ID,
    PULL_END_ID,
    ENERGY_ID
};

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
    int max_energy_per_round;
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
    char content[20];
} Message;


int load_config(const char *filename, Config *config);

#endif // CONFIG_H
