#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "config.h"
#include "sem.h"

int main() {
    int shm_id = atoi(getenv("SHM_ID")); // Shared memory ID passed via environment
    int sem= semget(1234, 1, 0666 );
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);


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
            printf("File calculator: calculate file %d\n", shared_data->total_calculated_files + shared_data->total_unprocessed_files);

            // Send message to Mover
            Message msg;
            msg.mtype = 1;  // Message type for movers
            strncpy(msg.file_number, file_number, 10);

            msgsnd(msgid, &msg, sizeof(msg), 0);
            shared_data->total_calculated_files++;
        }

        sem_post(sem);
        sleep(rand() % 3 + 1); // Simulate random delay
    }

    shmdt(shared_data);
    return 0;
}
