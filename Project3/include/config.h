#ifndef CONFIG_H
#define CONFIG_H

#define SIZE_OF_RESISTANT_GROUP 200
#define SIZE_OF_STATE 200
#define FIFO_NAME "/tmp/project3_fifo_public"

enum {
    Social_Type,
    Military_Type
};

enum {
    SPY,
    NORMAL
};

enum {
    KILLED,
    CAUGHT,
    INJURED_SEVERELY,
    INJURED_LIGHTLY,
    HEALTHY
};

enum {
    COUNTER_ESPIONAGE_AGENCY,
    RESISTANT_GROUP,
    ENEMY,
    CITIZEN
};

typedef struct {
    int number_of_member_in_counter_espionage_agency;
    int max_number_member_in_resistant_group;
    int min_number_member_in_resistant_group;
    int time_create_resistant_group;
    int max_time_to_rejoin_resistant_group;
    int suspicion_rate_threshold;
    int killed_resistance_members_threshold;
    int injured_resistance_members_threshold;
    int killed_caught_counter_members_threshold;
    int min_number_of_citizen;
    int max_number_of_citizen;
    int min_probability_to_attack;
} Config;

typedef struct {
    Config config;
    int simulation_status;
    int array_counter;
    int communication_pid[SIZE_OF_RESISTANT_GROUP][3];
    int number_of_killed_resistant_group;
    int number_of_injured_resistant_group;
    int number_of_killed_caught_counter_espionage_agency;
} SharedMemory;

typedef struct {
    long message_type;
    int group_id;
    int citizen_id;
    int time_spent;
    int the_spy;
    char suspicion_rates[SIZE_OF_STATE];
} Message_Queue;

typedef struct {
    int type;
    int id;
    char content[20];
} Information;



int load_config(const char *filename, Config *config);
void print_config(const Config *config);


#endif // CONFIG_H
