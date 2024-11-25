#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include "gui.h"
#include "config.h"

void create_process(const char *program, int shm_id, sem_t *sem, int count);

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
    sem_t *sem;

    // Initialize shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    memset(shared_data, 0, sizeof(SharedData));
    shared_data->simulation_running = 1;

    // Initialize semaphore
    sem = sem_open("/sim_sem", O_CREAT, 0644, 1);


    create_process("./bin/file_generator", shm_id, sem, config.num_file_generators);
    create_process("./bin/file_calculator",shm_id, sem, config.num_file_calculators);


    // Monitor simulation and terminate after runtime limit
    sleep(config.runtime_limit);
    shared_data->simulation_running = 0;
    while (wait(NULL) > 0);

    // Cleanup resources
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    sem_close(sem);
    sem_unlink("/sim_sem");

    printf("Simulation ended.\n");
    return 0;
}


void create_process(const char *program, int shm_id, sem_t *sem, int count) {
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


