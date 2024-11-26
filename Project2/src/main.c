#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include "gui.h"
#include "config.h"
#include "sem.h"

void create_process(const char *program, int shm_id, int count);

int main(int argc, char** argv) {
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


    int shm_id;
    SharedData *shared_data;

    // Initialize shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    memset(shared_data, 0, sizeof(SharedData));
    shared_data->simulation_running = 1;
    shared_data->config = config; // Add this lin

    // Initialize semaphore
    init_semaphore();

    create_process("./bin/file_generator", shm_id, config.num_file_generators);
    create_process("./bin/file_calculator",shm_id, config.num_file_calculators);
    create_process("./bin/file_mover",shm_id, config.num_file_movers);
    create_process("./bin/file_type1_inspector",shm_id, config.num_type1_inspectors);
    create_process("./bin/file_type2_inspector",shm_id, config.num_type2_inspectors);
    create_process("./bin/file_type3_inspector",shm_id, config.num_type3_inspectors);


    // Monitor simulation and terminate after runtime limit
    sleep(config.runtime_limit);
    shared_data->simulation_running = 0;

    while (wait(NULL) > 0);

    printf("Simulation complete\n");
    printf("Total generated files: %d\n", shared_data->total_generated_files);
    printf("Total calculated files: %d\n", shared_data->total_calculated_files);
    printf("Total unprocessed files: %d\n", shared_data->total_unprocessed_files);
    printf("Total backup files: %d\n", shared_data->total_backup_files);
    printf("Total deleted files: %d\n", shared_data->total_deleted_files);

    // Cleanup resources
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);



    return 0;
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


