#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int num_file_generators;
    int num_file_calculators;
    int num_file_movers;
    int num_type1_inspectors;
    int num_type2_inspectors;
    int num_type3_inspectors;
    int default_rows;
    int default_columns;
    int miss_percentage;
    int file_age_limit;
    int processed_limit;
    int unprocessed_limit;
    int backup_limit;
    int delete_limit;
    int runtime_limit;
    int value_range_min;
    int value_range_max;
} Config;

int load_config(const char *filename, Config *config);
void print_config(const Config *config);

#endif // CONFIG_H
