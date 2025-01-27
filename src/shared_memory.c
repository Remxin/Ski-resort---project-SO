#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "shared_memory.h"


int init_shared_memory() {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid != -1) {
        SharedData* data = (SharedData*)shmat(shmid, NULL, 0);
        if (data != (void*)-1) {
            // Initialize data
            memset(data, 0, sizeof(SharedData));
            
            // Initialize platform semaphores
            sem_init(&data->platform.platform_capacity, 1, MAX_PLATFORM_CAPACITY);
            for(int i = 0; i < 4; i++) {
                sem_init(&data->platform.lower_gates[i], 1, 1);
            }
            data->is_running = 1;
            data->is_paused = 0;
            data->lower_ready = 1;
            data->upper_ready = 1;
            data->platform.lower_platform_count = 0;
            data->platform.upper_platform_count = 0;

            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&data->queue_mutex, &attr);
                    
            
            shmdt(data);
        }
    }
    return shmid;
}

SharedData* attach_shared_memory(int shmid) {
    return (SharedData*)shmat(shmid,  NULL, 0);
}

void detach_shared_memory(SharedData* data) {
    if (data != NULL) {
        shmdt(data);
    }
}

void remove_shared_memory(int shmid) {
    shmctl(shmid, IPC_RMID, NULL);
}

int get_shorter_queue(SharedData* data) {

    if (data->queue1_length <= data->queue2_length) {
        return 1;
    }
    return 2;
}

void update_queue_length(SharedData* data, int queue_number, int delta) {
     pthread_mutex_lock(&data->queue_mutex);
    if (queue_number == 1) {
        data->queue1_length += delta;
        if (data->queue1_length < 0) data->queue1_length = 0;
    } else if (queue_number == 2) {
        data->queue2_length += delta;
        if (data->queue2_length < 0) data->queue2_length = 0;
    }

    pthread_mutex_unlock(&data->queue_mutex);
}