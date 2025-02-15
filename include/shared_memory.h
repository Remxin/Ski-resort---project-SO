#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>
#include "config.h"
#include "platform.h"  // Dodajemy załączenie platform.h tutaj

typedef struct {
    int queue1_length;
    int queue2_length;
    pthread_mutex_t queue_mutex;
    pid_t skiers[MAX_SKIERS];
    int skier_count;
    int is_running;
    int is_paused;
    int chair_array[MAX_CHAIRS];
    int lower_ready;
    int upper_ready;
    Platform platform;  // Dodajemy platformę jako część SharedData
} SharedData;

// Initialize shared memory
int init_shared_memory();

// Attach to shared memory
SharedData* attach_shared_memory(int shmid);

// Detach from shared memory
void detach_shared_memory(SharedData* data);

// Remove shared memory
void remove_shared_memory(int shmid);

// Get queue with shorter length
int get_shorter_queue(SharedData* data);

// Update queue length
void update_queue_length(SharedData* data, int queue_number, int delta);

#endif