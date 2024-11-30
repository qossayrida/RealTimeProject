#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "gui.h"
#include "config.h"
#include "sem.h"



void handle_alarm();
void create_process(const char *program, int shm_id, int count);
void try();

Information info_buffer[50];
SharedData *shared_data;
int fifo_fd, shm_id;

int main(int argc, char** argv) {
    try();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("File Management Simulation Configuration");

    init();
    glutDisplayFunc(display_config);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    while(menu_state != 2) {
        glutMainLoopEvent();
    }

    print_config(&config);


    // Initialize shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    memset(shared_data, 0, sizeof(SharedData));
    shared_data->simulation_running = 1;
    shared_data->config = config; // Add this lin
    shared_data->list_of_averages = create_list_node();



    // Initialize semaphore
    init_semaphore();

    if ( mknod(FIFO_NAME, S_IFIFO | 0666, 0) < 0 ) {
        perror(FIFO_NAME);
        exit(1);
    }

    fifo_fd = open(FIFO_NAME, O_RDONLY|O_NONBLOCK);

    signal(SIGALRM, handle_alarm);
    printf("Main: Maximum time set to %d seconds\n", config.runtime_limit);
    alarm(config.runtime_limit);

    create_process("./bin/file_generator", shm_id, config.num_file_generators);
    create_process("./bin/file_calculator",shm_id, config.num_file_calculators);
    create_process("./bin/file_mover",shm_id, config.num_file_movers);
    create_process("./bin/file_type1_inspector",shm_id, config.num_type1_inspectors);
    create_process("./bin/file_type2_inspector",shm_id, config.num_type2_inspectors);
    create_process("./bin/file_type3_inspector",shm_id, config.num_type3_inspectors);


    while(shared_data->simulation_running) {
        int bytes_read = read(fifo_fd, info_buffer, sizeof(info_buffer));
        int num_info;

        if (bytes_read != -1) {
            num_info = bytes_read / sizeof(Information);
            printf("Main: Received %d information\n", num_info);
        } else {
            num_info = 0;
        }

        for(int i = 0; i < num_info; i++) {
            if (info_buffer[i].type == FILE_GENERATOR) {
                printf("Main: File Generator %d generated file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
            } else if (info_buffer[i].type == FILE_CALCULATOR) {
                printf("Main: File Calculator %d calculated file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
            } else if (info_buffer[i].type == FILE_MOVER) {
                printf("Main: File Mover %d moved file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
            } else if (info_buffer[i].type == TYPE1_INSPECTOR) {
                printf("Main: Type 1 Inspector %d inspected file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
            } else if (info_buffer[i].type == TYPE2_INSPECTOR) {
                printf("Main: Type 2 Inspector %d inspected file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
            } else if (info_buffer[i].type == TYPE3_INSPECTOR) {
                printf("Main: Type 3 Inspector %d inspected file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
            }
        }
        if(shared_data->total_calculated_files >= shared_data->config.max_processed_file||
                shared_data->total_unprocessed_files >= shared_data->config.max_not_processed_file||
                shared_data->total_backup_files >= shared_data->config.max_backup_file||
                shared_data->total_deleted_files >= shared_data->config.max_deleted_file)
        { // Check if any of the limits are reached
            shared_data->simulation_running = 0;
        }

    }

    while (wait(NULL) > 0);
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    system("rm /tmp/file_fifo_public");
    printf("Main: Fifo removed\n");


    return 0;
}

void handle_alarm() {
    printf("Simulation complete\n");
    printf("Total generated files: %d\n", shared_data->total_generated_files);
    printf("Total calculated files: %d\n", shared_data->total_calculated_files);
    printf("Total unprocessed files: %d\n", shared_data->total_unprocessed_files);
    printf("Total backup files: %d\n", shared_data->total_backup_files);
    printf("Total deleted files: %d\n", shared_data->total_deleted_files);
    printf("Minimum average: %.2f in file %s\n", shared_data->min_average, shared_data->min_file);
    printf("Maximum average: %.2f in file %s\n", shared_data->max_average, shared_data->max_file);
    shared_data->simulation_running = 0;
}


void create_process(const char *program, int shm_id, int count) {
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            char shm_id_str[10];
            sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
            setenv("SHM_ID", shm_id_str, 1);  // Set SHM_ID as an environment variable
            execlp(program, program, NULL);  // Replace process with new program
            perror("execlp failed");
            exit(1); // Exit if execlp fails
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
    }
}



void try() {
    ListNodePtr outer_linked_list = create_list_node();


    AvgNodePtr avg_list = create_avg_node();
    add_avg_node(avg_list,1, 5.5);
    add_avg_node(avg_list,2, 6.5);

    add_list_node(outer_linked_list, avg_list);



    avg_list = create_avg_node();
    add_avg_node(avg_list,2, 7.5);
    add_avg_node(avg_list,4, 8.5);
    add_list_node(outer_linked_list, avg_list);


    print_linked_list(outer_linked_list);
}

