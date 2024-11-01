// src/main.c
#include <stdio.h>
#include "config.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    Config config;
    if (load_config(argv[1], &config) != 0) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    printf("Configuration Loaded:\n");
    printf("Max Score: %d\n", config.max_score);
    printf("Max Time: %d\n", config.max_time);
    printf("Initial Energy - Team A: %d\n", config.initial_energy_team_a);
    printf("Initial Energy - Team B: %d\n", config.initial_energy_team_b);
    printf("Initial Energy - Team C: %d\n", config.initial_energy_team_c);
    printf("Stabilization Time Min: %d\n", config.stabilization_time_min);
    printf("Stabilization Time Max: %d\n", config.stabilization_time_max);
    printf("Pull Time Min: %d\n", config.pull_time_min);
    printf("Pull Time Max: %d\n", config.pull_time_max);

    return 0;
}
