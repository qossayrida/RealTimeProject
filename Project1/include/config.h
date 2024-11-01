// include/config.h
#ifndef CONFIG_H
#define CONFIG_H

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

int load_config(const char *filename, Config *config);

#endif // CONFIG_H
