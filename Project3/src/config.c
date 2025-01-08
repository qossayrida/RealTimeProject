#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "config.h"
#include <semaphore.h>

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open configuration file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue; // Skip comments or empty lines

        char key[50];
        int value;
        if (sscanf(line, "%49[^=]=%d", key, &value) == 2) {
            if (strcmp(key, "time_create_resistant_group") == 0) config->time_create_resistant_group = value;
            else if (strcmp(key, "max_number_member_in_resistant_group") == 0) config->max_number_member_in_resistant_group = value;
            else if (strcmp(key, "min_number_member_in_resistant_group") == 0) config->min_number_member_in_resistant_group = value;
        }
    }

    fclose(file);
    return 0;
}

void print_config(const Config *config) {
    printf("Configuration:\n");
    printf("    Time to create resistant group: %d\n", config->time_create_resistant_group);
    printf("    Max number of members in resistant group: %d\n", config->max_number_member_in_resistant_group);
    printf("    Min number of members in resistant group: %d\n", config->min_number_member_in_resistant_group);
}
