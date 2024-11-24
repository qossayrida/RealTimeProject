#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

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
            if (strcmp(key, "num_file_generators") == 0) config->num_file_generators = value;
            else if (strcmp(key, "num_file_calculators") == 0) config->num_file_calculators = value;
            else if (strcmp(key, "num_file_movers") == 0) config->num_file_movers = value;
            else if (strcmp(key, "num_type1_inspectors") == 0) config->num_type1_inspectors = value;
            else if (strcmp(key, "num_type2_inspectors") == 0) config->num_type2_inspectors = value;
            else if (strcmp(key, "num_type3_inspectors") == 0) config->num_type3_inspectors = value;
            else if (strcmp(key, "default_rows") == 0) config->default_rows = value;
            else if (strcmp(key, "default_columns") == 0) config->default_columns = value;
            else if (strcmp(key, "miss_percentage") == 0) config->miss_percentage = value;
            else if (strcmp(key, "file_age_limit") == 0) config->file_age_limit = value;
            else if (strcmp(key, "processed_limit") == 0) config->processed_limit = value;
            else if (strcmp(key, "unprocessed_limit") == 0) config->unprocessed_limit = value;
            else if (strcmp(key, "backup_limit") == 0) config->backup_limit = value;
            else if (strcmp(key, "delete_limit") == 0) config->delete_limit = value;
            else if (strcmp(key, "runtime_limit") == 0) config->runtime_limit = value;
            else if (strcmp(key, "value_range_min") == 0) config->value_range_min = value;
            else if (strcmp(key, "value_range_max") == 0) config->value_range_max = value;
        }
    }

    fclose(file);
    return 0;
}
