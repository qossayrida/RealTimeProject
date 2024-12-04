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


#define MAX_NODES 1024


typedef struct Node {
    int data_head_offset;     // Offset to the head of the inner linked list
    int next_offset;          // Offset to the next inner node
    int filenumber;
} Node;

typedef struct InnerNode {
    float data;
    int rows;
    int next_offset; // Offset to the next node (relative to the base of shared memory)
} InnerNode;


#define MAX_PATH_LENGTH 40

typedef struct {
    int num_file_generators;
    int num_file_calculators;
    int num_file_movers;
    int num_type1_inspectors;
    int num_type2_inspectors;
    int num_type3_inspectors;
    int default_upper_rows;
    int default_lower_rows;
    int default_upper_columns;
    int default_lower_columns;
    int miss_percentage;
    int file_age_limit;
    int processed_limit;
    int backup_limit;
    int runtime_limit;
    int value_range_min;
    int value_range_max;
    int max_processed_file;
    int max_not_processed_file;
    int max_backup_file;
    int max_deleted_file;
    int min_generate_time;
    int max_generate_time;
    int max_wait_calculator_time;
    int min_wait_calculator_time;
} Config;

typedef struct {
    int total_generated_files;
    int total_calculated_files;
    int total_unprocessed_files;
    int total_backup_files;
    int total_deleted_files;
    float min_average;
    float max_average;
    Config config;
    char max_file[100];
    char min_file[100];
    int simulation_running; // 1 if running, 0 if terminated
    int avg_file[100][10];
    int rows[100][10];
    int head_offset;      // Offset to the head of the linked list
    int in;
    char memory[1];

} SharedData;

typedef struct {
    long mtype;           // Message type
    char file_number[10]; // File path
} Message;

typedef struct {
    int type;
    int sender_id;
    char content[30];
} Information;

#define MAX_FILES 9999

typedef struct {
    int number[6][MAX_FILES];
    float x[MAX_FILES];
    float y[MAX_FILES];
    int file_counts[6];
    float colors[MAX_FILES][3];
} DirectoryData;

int load_config(const char *filename, Config *config);

void print_config(const Config *config);

void send_info(int fifo, int type, int sender_id, char *content);

void timer();

void generate_file(int generator_id, int file_number);

void create_process(const char *program, int shm_id, int count);

void traverse_list(SharedData *shm);

void initialize_shared_memory(SharedData *shm);

void add_node(SharedData *shm,int filenumber);

void add_internal_node(SharedData *shm,  int rows, float average, int o);

void draw_min_max(float min, float max, char *min_file, char *max_file,int line);


#endif // CONFIG_H
