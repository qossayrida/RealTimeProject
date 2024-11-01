#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configuration structure
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

// Function to parse the config file
int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open configuration file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;

        char key[50];
        int value;

        if (sscanf(line, "%49[^=]=%d", key, &value) == 2) {
            if (strcmp(key, "max_score") == 0)
                config->max_score = value;
            else if (strcmp(key, "max_time") == 0)
                config->max_time = value;
            else if (strcmp(key, "initial_energy_team_a") == 0)
                config->initial_energy_team_a = value;
            else if (strcmp(key, "initial_energy_team_b") == 0)
                config->initial_energy_team_b = value;
            else if (strcmp(key, "initial_energy_team_c") == 0)
                config->initial_energy_team_c = value;
            else if (strcmp(key, "stabilization_time_min") == 0)
                config->stabilization_time_min = value;
            else if (strcmp(key, "stabilization_time_max") == 0)
                config->stabilization_time_max = value;
            else if (strcmp(key, "pull_time_min") == 0)
                config->pull_time_min = value;
            else if (strcmp(key, "pull_time_max") == 0)
                config->pull_time_max = value;
        }
    }

    fclose(file);
    return 0;
}

// Main function demonstrating usage
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

    // Output loaded configuration for verification
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

    // Continue with the rest of the game setup and logic using `config`

    return 0;
}
