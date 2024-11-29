#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
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