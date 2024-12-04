#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "config.h"
#include "sem.h"

int fifo_fd;
char *read_line(FILE *file);
int extractFileNumber(const char *fileName);

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem= semget(1234, 1, 0666 );
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    fifo_fd = open(FIFO_NAME, O_WRONLY);


    // Open message queue
    key_t key = ftok("/tmp/file_queue", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    while (shared_data->simulation_running&&shared_data->total_calculated_files<shared_data->config.max_processed_file) {

        sleep((rand() % (shared_data->config.max_wait_calculator_time - shared_data->config.min_wait_calculator_time + 1)) +
              shared_data->config.min_wait_calculator_time);

        sem_wait(sem);
        char file_number[MAX_PATH_LENGTH];
        snprintf(file_number, MAX_PATH_LENGTH, "%d", shared_data->total_calculated_files + shared_data->total_unprocessed_files);

        char source[MAX_PATH_LENGTH*2];
        snprintf(source, MAX_PATH_LENGTH*2, "%s%s.csv", HOME_DIR, file_number);

        if (access(source, F_OK) == 0) {
            add_node(shared_data,shared_data->total_calculated_files + shared_data->total_unprocessed_files );
            send_info(fifo_fd, FILE_CALCULATOR, getpid(), file_number);

            FILE *file = fopen(source, "r");
            if (!file){
                perror("Failed to open file");
                continue;
            }
            int columnCount = 0;
            double sums[shared_data->config.default_upper_columns];
            int rows[shared_data->config.default_upper_rows]; // Declare the array
            for (int i = 0; i < shared_data->config.default_upper_rows; i++) {
                rows[i] = 0; // Initialize each element to 0
            }
            for (int i = 0; i < shared_data->config.default_upper_columns; i++) {
                sums[i] = 0; // Initialize each element to 0
            }

            char line[1024];

            // Read each line from the file
            try_again:
            // Read each line from the file
            while (fgets(line, sizeof(line), file)){

                char* token = strtok(line, ",");
                columnCount = 0;

                while (token != NULL) {

                    if (strcmp(token, "-") == 0) {
                        // Skip this token (i.e., do not add it to the sum)
                        token = strtok(NULL, ",");
                        columnCount++;
                        continue;
                    }

                    sums[columnCount] += atof(token);
                    rows[columnCount]+=1;
                    columnCount++;
                    token = strtok(NULL, ",");


                }
                memset(line, 0, sizeof(line));
            }



            if (columnCount==0){
                printf("%d",access(source, F_OK));
                if (feof(file)){
                    rewind(file); // Resets the pointer to the beginning
                    goto try_again;
                }
            }

            printf("column count %d\n", columnCount);
            printf("file %s\n", file_number);


            for (int i = 0; i < columnCount; i++) {
                shared_data->avg_file[shared_data->total_calculated_files-1][i] = sums[i] / rows[i];
                shared_data->rows[shared_data->total_calculated_files-1][i] = rows[i];
                add_internal_node(shared_data,rows[i], sums[i] / rows[i],shared_data->head_offset);

                if(shared_data->max_average < sums[i] / rows[i]){
                    shared_data->max_average = sums[i] / rows[i];
                    snprintf(shared_data->max_file, sizeof(shared_data->max_file), "%s col %d", file_number,i);
                }
                if(shared_data->min_average ==0){
                    shared_data->min_average = sums[i] / rows[i];
                }
                if(shared_data->min_average > sums[i] / rows[i]){
                    shared_data->min_average = sums[i] / rows[i];
                    snprintf(shared_data->min_file, sizeof(shared_data->min_file), "%s col %d", file_number,i);
                }
            }
            fclose(file);

            Message msg;
            msg.mtype = 1;  // Message type for movers
            strncpy(msg.file_number, file_number, 10);
            msgsnd(msgid, &msg, sizeof(msg), 0);
            shared_data->total_calculated_files++;
        }

        sem_post(sem);
    }

    sem_wait(sem);
    shared_data->coutine_move--;
    printf("File calculator: %d\n",shared_data->coutine_move);
    sem_post(sem);
    shmdt(shared_data);
    return 0;
}

char *read_line(FILE *file) {
    if (file == NULL) {
        return NULL; // Return NULL if the file is not valid
    }

    int length = 0;
    char buffer[1024];


    int ch;
    while ((ch = getc(file)) != EOF && ch != '\n') {
        buffer[length++] = ch;
    }

    char *line = malloc(sizeof(char) * length+2); // Allocate memory for the line

    if (line == NULL) {
        printf("Failed to allocate memory for line\n");
        return NULL; // Return NULL if memory allocation fails
    }

    // Check if we read anything before EOF
    if (length == 0 && ch == EOF) {
        free(line); // Free allocated memory
        return NULL;
    }

    buffer[length] = '\0'; // Null-terminate the string
    strncpy(line, buffer, length + 1); // Copy buffer to line
    return line;
}

int extractFileNumber(const char *fileName) {
    const char *last_slash = strrchr(fileName, '/'); // Find the last '/'
    if (last_slash != NULL) {
        int fileNumber;
        if (sscanf(last_slash + 1, "%d", &fileNumber) == 1){
            return fileNumber;
        }
    }
    return -1; // Return -1 if extraction fails
}
