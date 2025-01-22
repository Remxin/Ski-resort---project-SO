#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include "shared_memory.h"


int init_shared_memory() {
   int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
   if (shmid != -1) {
       SharedData* data = (SharedData*)shmat(shmid, NULL, 0);
       if (data != (void*)-1) {
           memset(data, 0, sizeof(SharedData));
           
           // Inicjalizacja semaforów platformy
           sem_init(&data->platform.platform_capacity, 1, MAX_PLATFORM_CAPACITY);
           for(int i = 0; i < 4; i++) {
               sem_init(&data->platform.lower_gates[i], 1, 1);
           }

           // Semafor licznika
           data->platform.counter_sem = sem_open("/platform_count", O_CREAT, 0644, 1);
           data->platform.lower_platform_count = 0;
           data->platform.upper_platform_count = 0;

           printf("Shared memory initialized with platform capacity: %d\n", MAX_PLATFORM_CAPACITY);
           shmdt(data);
       }
   }
   return shmid;
}

int get_shared_memory_id(void) {
    return shmget(SHM_KEY, sizeof(SharedData), 0);
}

SharedData* attach_shared_memory(int shmid) {
    return (SharedData*)shmat(shmid, NULL, 0);
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
    if (queue_number == 1) {
        data->queue1_length += delta;
        if (data->queue1_length < 0) data->queue1_length = 0;
    } else if (queue_number == 2) {
        data->queue2_length += delta;
        if (data->queue2_length < 0) data->queue2_length = 0;
    }
}