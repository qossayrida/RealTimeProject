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

    printf("Max Score: %d\n", config.max_score);
    printf("Max Time: %d\n", config.max_time);
    // Initialize other processes and start game logic

    return 0;
}
