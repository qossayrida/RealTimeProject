// include/config.h
#ifndef CONFIG_H
#define CONFIG_H


#define JUMP_ID 0
#define PULL_ID 1
#define ENERGY_ID 2

typedef struct {
    int max_score;
    int max_time;
    int initial_energy_team_a;
    int initial_energy_team_b;
    int initial_energy_team_c;
    int stabilization_time_min;
    int stabilization_time_max;
    int pull_time_min;
    int pull_time_max;
} Config;


typedef struct {
    int team_id;
    int score;
    int turn;
    int deceased [3];
    pid_t players[3];
} Team;


typedef struct {
    int type;
    char content[50];
} Message;


int load_config(const char *filename, Config *config);

#endif // CONFIG_H
