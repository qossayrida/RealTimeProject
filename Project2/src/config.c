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
    printf(".................................................\n");
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
            else if (strcmp(key, "default_upper_rows") == 0) config->default_upper_rows = value;
            else if (strcmp(key, "default_lower_rows") == 0) config->default_lower_rows = value;
            else if (strcmp(key, "default_upper_columns") == 0) config->default_upper_columns = value;
            else if (strcmp(key, "default_lower_columns") == 0) config->default_lower_columns = value;
            else if (strcmp(key, "miss_percentage") == 0) config->miss_percentage = value;
            else if (strcmp(key, "file_age_limit") == 0) config->file_age_limit = value;
            else if (strcmp(key, "processed_limit") == 0) config->processed_limit = value;
            else if (strcmp(key, "backup_limit") == 0) config->backup_limit = value;
            else if (strcmp(key, "runtime_limit") == 0) config->runtime_limit = value;

            else if (strcmp(key, "value_range_min") == 0) config->value_range_min = value;
            else if (strcmp(key, "value_range_max") == 0) config->value_range_max = value;

            else if (strcmp(key, "max_processed_file") == 0) config->max_processed_file = value;
            else if (strcmp(key, "max_not_processed_file") == 0) config->max_not_processed_file = value;
            else if (strcmp(key, "max_backup_file") == 0) config->max_backup_file = value;
            else if (strcmp(key, "max_deleted_file") == 0) config->max_deleted_file = value;

            else if (strcmp(key, "min_generate_time") == 0) config->min_generate_time = value;
            else if (strcmp(key, "max_generate_time") == 0) config->max_generate_time = value;



        }
    }

    fclose(file);
    return 0;
}

void print_config(const Config *config) {
    printf("Config Settings:\n");
    printf("  num_file_generators: %d\n", config->num_file_generators);
    printf("  num_file_calculators: %d\n", config->num_file_calculators);
    printf("  num_file_movers: %d\n", config->num_file_movers);
    printf("  num_type1_inspectors: %d\n", config->num_type1_inspectors);
    printf("  num_type2_inspectors: %d\n", config->num_type2_inspectors);
    printf("  num_type3_inspectors: %d\n", config->num_type3_inspectors);
    printf("  default_upper_rows: %d\n", config->default_upper_rows);
    printf("  default_lower_rows: %d\n", config->default_lower_rows);
    printf("  default_upper_columns: %d\n", config->default_upper_columns);
    printf("  default_lower_columns: %d\n", config->default_lower_columns);
    printf("  miss_percentage: %d\n", config->miss_percentage);
    printf("  file_age_limit: %d\n", config->file_age_limit);
    printf("  processed_limit: %d\n", config->processed_limit);
    printf("  backup_limit: %d\n", config->backup_limit);
    printf("  runtime_limit: %d\n", config->runtime_limit);
    printf("  value_range_min: %d\n", config->value_range_min);
    printf("  value_range_max: %d\n", config->value_range_max);
    printf("  max_processed_file: %d\n", config->max_processed_file);
    printf("  max_not_processed_file: %d\n", config->max_not_processed_file);
    printf("  max_backup_file: %d\n", config->max_backup_file);
    printf("  max_deleted_file: %d\n", config->max_deleted_file);
}

void send_info(int fifo ,int type ,int sender_id, char *content){
    Information info;
    info.type = type;
    info.sender_id = sender_id;
    snprintf(info.content, sizeof(info.content), "%s", content);
    if(write(fifo, &info, sizeof(Information))== -1)
        perror("Failed to write team data to referee");
}

ListNodePtr create_list_node() {
    ListNodePtr new_node = (ListNodePtr)malloc(sizeof(ListNode));
    new_node->list_of_average = NULL;
    new_node->next = NULL;
    return new_node;
}

AvgNodePtr create_avg_node() {
    AvgNodePtr new_node = (AvgNodePtr)malloc(sizeof(AvgNode));
    new_node->num_rows = 0;
    new_node->average = 0;
    new_node->next = NULL;
    return new_node;
}

void add_list_node(ListNodePtr outer_linked_list, AvgNodePtr avg_list) {
    ListNodePtr new_node = create_list_node();
    new_node->list_of_average = avg_list;

    ListNodePtr current = outer_linked_list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

void add_avg_node(AvgNodePtr avg_list,int rows ,float new_avg) {
    AvgNodePtr new_node = create_avg_node();
    new_node->num_rows = rows;
    new_node->average = new_avg;

    AvgNodePtr current = avg_list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}


void print_linked_list(ListNodePtr outer_linked_list) {
    ListNodePtr current_list_node = outer_linked_list->next;
    int i = 0;
    while (current_list_node != NULL) {
        printf("List Node %d :\n", i++);
        AvgNodePtr current_avg_node = current_list_node->list_of_average->next;
        while (current_avg_node != NULL) {
            printf("  Row: %d    Average: %.2f\n",current_avg_node->num_rows, current_avg_node->average);
            current_avg_node = current_avg_node->next;
        }
        current_list_node = current_list_node->next;
    }
}







void add_node(SharedData *shm) {



    // Find a free slot in the memory pool
    int offset = 0;
    Node *node = (Node *)(shm->memory);

    //printf("head offset %d\n", shm->head_offset);
    //printf("offset %d\n", node->next_offset);

    if (shm->head_offset == -1)
    {
        node = (Node *)(shm->memory + offset);
        if (offset >= MAX_NODES * sizeof(Node)) {
            printf("No space left in shared memory for a new node.\n");
            return;
        }}
    else {
        offset=shm->head_offset+sizeof(Node);
        node = (Node *)(shm->memory+(offset));
    }


    // Add the new
    node->data_head_offset=-1;   // Offset to the head of the inner linked list


    node->next_offset = shm->head_offset; // Point to the current head node


    shm->head_offset = offset;







}


void traverse_list(SharedData *shm) {

    int offset = shm->head_offset;
    int i=1;

    while (offset != -1) {
        Node *node = (Node *)(shm->memory + offset);
        //sleep(3);

        int offset2 = node->data_head_offset;


        while (offset2 != -1) {
            InnerNode *in_node = (InnerNode *)(shm->memory + offset2);
            printf("%dNode data: %f\n", i++,in_node->data);
            offset2 = in_node->next_offset;
        }
        offset = node->next_offset;
    }

}



void add_internal_node(SharedData *shm, int rows,float average,int o) {

    // Find a free slot in the memory pool
    int offset = shm->in;

    Node *node = (Node *)(shm->memory+o);
    InnerNode *in_node = (InnerNode *)(shm->memory + MAX_NODES * sizeof(Node));




    if (node->data_head_offset == -1)
    {
        in_node = (InnerNode *)(shm->memory + offset);
    }
    else {
        offset=node->data_head_offset+sizeof(InnerNode);
        in_node = (InnerNode *)(shm->memory+(offset));
    }

    // Add the new
    in_node->data=average;    // Offset to the head of the inner linked list
    in_node->rows=rows;


    in_node->next_offset = node->data_head_offset;// Point to the current head node


    node->data_head_offset = offset;
    shm->in =offset+12;








}
void initialize_shared_memory(SharedData *shm) {
    if (shm->head_offset == 0) {
        shm->head_offset = -1; // No nodes in the list yet
    }
}