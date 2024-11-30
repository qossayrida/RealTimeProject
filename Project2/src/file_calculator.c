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

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem= semget(1234, 1, 0666 );
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    fifo_fd = open(FIFO_NAME, O_WRONLY);


    // Open message queue
    key_t key = ftok("queuefile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    while (shared_data->simulation_running) {
        sem_wait(sem);


        char file_number[MAX_PATH_LENGTH];
        snprintf(file_number, MAX_PATH_LENGTH, "%d", shared_data->total_calculated_files + shared_data->total_unprocessed_files);

        char source[MAX_PATH_LENGTH*2];
        snprintf(source, MAX_PATH_LENGTH*2, "%s%s.csv", HOME_DIR, file_number);


        if (access(source, F_OK) != -1) {
            send_info(fifo_fd, FILE_CALCULATOR, getpid(), file_number);

            // Send message to Mover
            Message msg;
            msg.mtype = 1;  // Message type for movers
            strncpy(msg.file_number, file_number, 10);
            msgsnd(msgid, &msg, sizeof(msg), 0);
            shared_data->total_calculated_files++;


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
            while (fgets(line, sizeof(line), file)) {

                char* token = strtok(line, ",");
                columnCount = 0;
                while (token != NULL) {
                    if (strcmp(token, "MISSING") == 0) {
                        // Skip this token (i.e., do not add it to the sum)
                        token = strtok(NULL, ",");
                        columnCount++;
                        continue;
                    }

                    // Convert the token to a double and add to the corresponding column sum
                    sums[columnCount] += atof(token);
                    rows[columnCount]+=1;
                    columnCount++;
                    token = strtok(NULL, ",");
                }

            }

            for (int i = 0; i < columnCount; i++) {
                printf("Average of column %d: %.2f and number of rows are %d\n", i + 1, sums[i] / rows[i],rows[i]);
                shared_data->avg_file[shared_data->total_calculated_files-1][i] = sums[i] / rows[i];
                shared_data->rows[shared_data->total_calculated_files-1][i] = rows[i];

                if(shared_data->max_average < sums[i] / rows[i]){
                    shared_data->max_average = sums[i] / rows[i];
                    snprintf(shared_data->max_file, sizeof(shared_data->max_file), "%s col %d", file_number,i);
                }
                if(shared_data->min_average ==0){
                    shared_data->min_average = sums[i] / rows[i];
                }
                if(shared_data->min_average > sums[i] / rows[i]){
                    shared_data->min_average = sums[i] / rows[i];
                    snprintf(shared_data->min_file, sizeof(shared_data->min_file), "%s.csv col %d", file_number,i);
                }
            }


//            AvgNodePtr avg_list = create_avg_node();
//            for (int i = 0; i < columnCount; i++){
//                add_avg_node(avg_list,rows[i], sums[i] / rows[i]);
//            }
//            add_list_node(shared_data->list_of_averages, avg_list);


            fclose(file);
        }

        sem_post(sem);
        sleep(rand() % 3 + 1); // Simulate random delay
    }

    shmdt(shared_data);
    return 0;
}
