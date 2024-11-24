#include <stdio.h>
#include "config.h"

Config config;

int main() {
    if (load_config("config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    printf("File Generators: %d\n", config.num_file_generators);
    printf("File Calculators: %d\n", config.num_file_calculators);
    printf("Rows: %d, Columns: %d\n", config.default_rows, config.default_columns);
    // Add further processing logic here.

    return 0;
}
 
