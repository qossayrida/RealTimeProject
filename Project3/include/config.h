#ifndef CONFIG_H
#define CONFIG_H


typedef struct {
    int time_create_resistant_group;
    int max_number_member_in_resistant_group;
    int min_number_member_in_resistant_group;
    int time_to_end;
} Config;

typedef struct {
    Config config;
} SharedMemory;


int load_config(const char *filename, Config *config);
void print_config(const Config *config);


#endif // CONFIG_H
