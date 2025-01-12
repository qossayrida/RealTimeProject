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
            if (strcmp(key, "number_of_member_in_counter_espionage_agency") == 0)
                config->number_of_member_in_counter_espionage_agency = value;
            if (strcmp(key, "time_create_resistant_group") == 0) config->time_create_resistant_group = value;
            else if (strcmp(key, "max_number_member_in_resistant_group") == 0)
                config->max_number_member_in_resistant_group = value;
            else if (strcmp(key, "min_number_member_in_resistant_group") == 0)
                config->min_number_member_in_resistant_group = value;
            else if (strcmp(key, "max_time_to_rejoin_resistant_group") == 0)
                config->max_time_to_rejoin_resistant_group = value;
            else if (strcmp(key, "suspicion_rate_threshold") == 0) config->suspicion_rate_threshold = value;
            else if (strcmp(key, "killed_resistance_members_threshold") == 0)
                config->killed_resistance_members_threshold = value;
            else if (strcmp(key, "injured_resistance_members_threshold") == 0)
                config->injured_resistance_members_threshold = value;
            else if (strcmp(key, "killed_caught_counter_members_threshold") == 0)
                config->killed_caught_counter_members_threshold = value;
            else if (strcmp(key, "min_number_of_citizen") == 0) config->min_number_of_citizen = value;
            else if (strcmp(key, "max_number_of_citizen") == 0) config->max_number_of_citizen = value;
            else if (strcmp(key, "min_probability_to_attack") == 0) config->min_probability_to_attack = value;
        }
    }

    fclose(file);
    return 0;
}

void print_config(const Config *config) {
    printf("Configuration:\n");
    printf("    Number of member in counter espionage agency: %d\n",
           config->number_of_member_in_counter_espionage_agency);
    printf("    Time to create resistant group: %d\n", config->time_create_resistant_group);
    printf("    Max number of members in resistant group: %d\n", config->max_number_member_in_resistant_group);
    printf("    Min number of members in resistant group: %d\n", config->min_number_member_in_resistant_group);
    printf("    Max time to rejoin resistant group: %d\n", config->max_time_to_rejoin_resistant_group);
    printf("    Suspicion rate threshold: %d\n", config->suspicion_rate_threshold);
    printf("    Killed resistance members threshold: %d\n", config->killed_resistance_members_threshold);
    printf("    Injured resistance members threshold: %d\n", config->injured_resistance_members_threshold);
    printf("    Killed or caught counter espionage agency members threshold: %d\n",
           config->killed_caught_counter_members_threshold);
    printf("    Min number of citizens: %d\n", config->min_number_of_citizen);
    printf("    Max number of citizens: %d\n", config->max_number_of_citizen);
    printf("    Min probability to attack: %d\n", config->min_probability_to_attack);
}

void send_info_to_gui(int fifo_fd, int type, int id, char *content) {
    Information info;
    info.type = type;
    info.id = id;
    snprintf(info.content, sizeof(info.content), "%s", content);
    write(fifo_fd, &info, sizeof(Information));
}