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

void timer();

void handle_alarm();

void create_process(const char *program, int shm_id, int count);

int extractFileNumber(const char *fileName);

void calculateFile(const char *file);

void generateFile(int senderID, const char *file);

void changeDirectory(int senderID, int fileNumber, int source, int destination);

void display();

Information info_buffer[50];
SharedData *shared_data;
int fifo_fd, shm_id;

DirectoryData directory_data = {0}; // Initialize directory data to empty
const char *directory_names[6] = {"Generated", "Home", "Processed", "Unprocessed", "Backup", "Deleted"};

int main(int argc, char **argv) {

    system("rm -rf Home Processed UnProcessed Backup");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("File Management Simulation Configuration");

    init();
    glutDisplayFunc(display_config);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    while (menu_state != 2) {
        glutMainLoopEvent();
    }

    print_config(&config);

    // Initialize shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(SharedData) + 100 * MAX_NODES * sizeof(Node), IPC_CREAT | 0666);
    shared_data = (SharedData *) shmat(shm_id, NULL, 0);
    memset(shared_data, 0, sizeof(SharedData));
    shared_data->simulation_running = 1;
    shared_data->config = config; // Add this lin
    shared_data->list_of_averages = create_list_node();
    shared_data->head_offset = -1;
    shared_data->in = MAX_NODES * sizeof(Node);

    // Initialize semaphore
    init_semaphore();

    if (mknod(FIFO_NAME, S_IFIFO | 0666, 0) < 0) {
        perror(FIFO_NAME);
        exit(1);
    }

    fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);

    signal(SIGALRM, handle_alarm);
    printf("Main: Maximum time set to %d seconds\n", config.runtime_limit);
    alarm(config.runtime_limit);

    create_process("./bin/file_generator", shm_id, config.num_file_generators);
    create_process("./bin/file_calculator", shm_id, config.num_file_calculators);
    create_process("./bin/file_mover", shm_id, config.num_file_movers);
    create_process("./bin/file_type1_inspector", shm_id, config.num_type1_inspectors);
    create_process("./bin/file_type2_inspector", shm_id, config.num_type2_inspectors);
    create_process("./bin/file_type3_inspector", shm_id, config.num_type3_inspectors);


    memset(directory_data.file_counts, 0, sizeof(directory_data.file_counts));
    glutDisplayFunc(display);
    glutPostRedisplay();
    // Start simulation
    glutIdleFunc(timer); // Use timer to update simulation
    glutMainLoop();      // Start OpenGL main loop

    return 0;
}

void timer() {
    int bytes_read = read(fifo_fd, info_buffer, sizeof(info_buffer));
    int num_info = (bytes_read != -1) ? bytes_read / sizeof(Information) : 0;

    if (num_info > 0) {
        printf("Timer: Received %d information.\n", num_info);
    }

    for (int i = 0; i < num_info; i++) {

        for (int j = 0; j < 6; j++) {
            printf("Directory %s has %d files.\n", directory_names[j], directory_data.file_counts[j]);
        }

        switch (info_buffer[i].type) {
            case FILE_GENERATOR:
                printf("Timer: File Generator %d generated file %s\n", info_buffer[i].sender_id,
                       info_buffer[i].content);
                generateFile(info_buffer[i].sender_id, info_buffer[i].content);
                break;
            case FILE_CALCULATOR:
                printf("Timer: File Calculator %d calculated file %s\n", info_buffer[i].sender_id,
                       info_buffer[i].content);
                calculateFile(info_buffer[i].content);
                break;
            case FILE_MOVER:
                printf("Timer: File Mover %d moved file %s\n", info_buffer[i].sender_id, info_buffer[i].content);
                changeDirectory(info_buffer[i].sender_id, atoi(info_buffer[i].content), 1, 2);
                break;
            case TYPE1_INSPECTOR:
                printf("Timer: Type 1 Inspector %d inspected file %s\n", info_buffer[i].sender_id,
                       info_buffer[i].content);
                changeDirectory(info_buffer[i].sender_id, atoi(info_buffer[i].content), 1, 3);
                break;
            case TYPE2_INSPECTOR:
                printf("Timer: Type 2 Inspector %d inspected file %s\n", info_buffer[i].sender_id,
                       info_buffer[i].content);
                changeDirectory(info_buffer[i].sender_id, atoi(info_buffer[i].content), 2, 4);
                break;
            case TYPE3_INSPECTOR:
                printf("Timer: Type 3 Inspector %d inspected file %s\n", info_buffer[i].sender_id,
                       info_buffer[i].content);
                changeDirectory(info_buffer[i].sender_id, atoi(info_buffer[i].content), 4, 5);
                break;
            default:
                break;
        }
    }

    // Check simulation termination conditions
    if (shared_data->total_calculated_files >= shared_data->config.max_processed_file ||
        shared_data->total_unprocessed_files >= shared_data->config.max_not_processed_file ||
        shared_data->total_backup_files >= shared_data->config.max_backup_file ||
        shared_data->total_deleted_files >= shared_data->config.max_deleted_file) {
        printf("Timer: Simulation completed. Stopping.\n");
        shared_data->simulation_running = 0;
        kill(getpid(), SIGALRM);
    }

    glutPostRedisplay(); // Trigger redraw
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
    //traverse_list(shared_data);
    glutIdleFunc(NULL);  // Stop idle function
    glutLeaveMainLoop(); // Exit GLUT main loop
    while (wait(NULL) > 0);
    traverse_list(shared_data);
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    system("rm /tmp/file_fifo_public");
    printf("Main: Fifo removed\n");
    printf("You can now close the window\n");
}

void create_process(const char *program, int shm_id, int count) {
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            char shm_id_str[10];
            sprintf(shm_id_str, "%d", shm_id); // Convert shm_id to string
            setenv("SHM_ID", shm_id_str, 1);   // Set SHM_ID as an environment variable
            execlp(program, program, NULL);    // Replace process with new program
            perror("execlp failed");
            exit(1); // Exit if execlp fails
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
    }
}

int extractFileNumber(const char *fileName) {
    const char *last_slash = strrchr(fileName, '/'); // Find the last '/'
    if (last_slash != NULL) {
        int fileNumber;
        if (sscanf(last_slash + 1, "%d", &fileNumber) == 1) // Extract the number
        {
            return fileNumber;
        }
    }
    return -1; // Return -1 if extraction fails
}

void changeDirectory(int senderID, int fileNumber, int source, int destination) {
    // If the file is moving from a source directory, remove it from there
    if (source != -1) {
        int source_index = -1;
        for (int i = 0; i < directory_data.file_counts[source]; i++) {
            if (directory_data.number[source][i] == fileNumber) {
                source_index = i;
                break;
            }
        }

        if (source_index != -1) {
            // Shift remaining files to maintain order in the source directory
            for (int i = source_index; i < directory_data.file_counts[source] - 1; i++) {
                directory_data.number[source][i] = directory_data.number[source][i + 1];
            }
            directory_data.file_counts[source]--; // Decrement source directory file count
        }
    }

    // Check if the file already exists in the destination directory
    for (int i = 0; i < directory_data.file_counts[destination]; i++) {
        if (directory_data.number[destination][i] == fileNumber) {
            return;
        }
    }

    // Add the file to the destination directory
    int dest_index = directory_data.file_counts[destination];
    directory_data.number[destination][dest_index] = fileNumber;
    directory_data.file_counts[destination]++; // Increment destination directory file count

    // Update position for visualization
    int files_per_column = 26; // Maximum files per column
    int column = dest_index / files_per_column;
    int row = dest_index % files_per_column;

    directory_data.x[fileNumber] = -1.0f + destination * 0.4f + column * 0.05f; // Adjust x position
    directory_data.y[fileNumber] = 1.0f - (row + 1) * 0.05f;

    glutPostRedisplay();
}

void generateFile(int senderID, const char *file) {
    int fileNumber = extractFileNumber(file);

    float r, g, b;
    get_color(senderID, &r, &g, &b);

    // Assign colors and properties
    directory_data.colors[fileNumber][0] = r;
    directory_data.colors[fileNumber][1] = g;
    directory_data.colors[fileNumber][2] = b;

    // Add the file to the "Generated" directory (index 0)
    int generated_dir_index = 0;
    directory_data.number[generated_dir_index][directory_data.file_counts[generated_dir_index]] = fileNumber;
    directory_data.file_counts[generated_dir_index]++;

    changeDirectory(senderID, fileNumber, -1, 1);
}

void calculateFile(const char *file) {
    //    int fileNumber = atoi(file);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the table (80% of the screen width)
    glViewport(0, 0, (int) (0.8 * glutGet(GLUT_WINDOW_WIDTH)), glutGet(GLUT_WINDOW_HEIGHT));
    draw_table();

    // Draw the chart (20% of the screen width)
    glViewport((int) (0.8 * glutGet(GLUT_WINDOW_WIDTH)), 0, (int) (0.2 * glutGet(GLUT_WINDOW_WIDTH)),
               glutGet(GLUT_WINDOW_HEIGHT));
    draw_chart();

    glutSwapBuffers();
}
