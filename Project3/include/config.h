#ifndef CONFIG_H
#define CONFIG_H

enum {
    Social_Type,
    Military_Type
};

enum {
    KILLED,
    CAUGHT,
    INJURED_SEVERELY,
    INJURED_LIGHTLY
};

typedef struct {
    int number_of_member_in_counter_espionage_agency;
    int max_number_member_in_resistant_group;
    int min_number_member_in_resistant_group;
    int time_create_resistant_group;
    int time_to_end;
} Config;

typedef struct {
    Config config;
} SharedMemory;

typedef struct {
    long message_type;
    int group_id;
    int citizen_id;
    int time_spent;
} Message_Queue;


int load_config(const char *filename, Config *config);
void print_config(const Config *config);


#endif // CONFIG_H
