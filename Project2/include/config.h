#ifndef CONFIG_H
#define CONFIG_H

#define FIFO_NAME "/tmp/file_fifo_public"

#define HOME_DIR "./Home/"
#define PROCESSED_DIR "./Processed/"
#define UNPROCESSED_DIR "./UnProcessed/"
#define BACKUP_DIR "./Backup/"

enum {
    FILE_GENERATOR,
    FILE_CALCULATOR,
    FILE_MOVER,
    TYPE1_INSPECTOR,
    TYPE2_INSPECTOR,
    TYPE3_INSPECTOR
};

#define MAX_PATH_LENGTH 40

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
    int backup_limit;
    int runtime_limit;
    int value_range_min;
    int value_range_max;
} Config;

typedef struct {
    int total_generated_files;
    int total_calculated_files;
    int total_unprocessed_files;
    int total_backup_files;
    int total_deleted_files;
    float min_average;
    float max_average;
    char min_avg_file[256];
    char max_avg_file[256];
    int simulation_running; // 1 if running, 0 if terminated
    Config config;
} SharedData;


typedef struct {
    long mtype;         // Message type
    char file_number[10];  // File path
} Message;

typedef struct {
    int type;
    int sender_id;
    char content[30];
} Information;

int load_config(const char *filename, Config *config);
void print_config(const Config *config);
void send_info(int fifo,int type ,int sender_id, char *content);

#endif // CONFIG_H
