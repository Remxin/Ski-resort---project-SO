#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "platform.h"

Platform* init_platform() {
    Platform* platform = malloc(sizeof(Platform));
    if (platform == NULL) {
        perror("Failed to allocate platform");
        return NULL;
    }
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&platform->count_mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);
    
    printf("Initializing platform with capacity %d\n", MAX_PLATFORM_CAPACITY);
    
    // Inicjalizacja semafora pojemności peronu
    if (sem_init(&platform->platform_capacity, 1, MAX_PLATFORM_CAPACITY) != 0) {
        perror("Failed to initialize platform capacity semaphore");
        free(platform);
        return NULL;
    }

    int sem_value;
    sem_getvalue(&platform->platform_capacity, &sem_value);
    printf("Initial platform capacity semaphore value: %d\n", sem_value);
    
    // Inicjalizacja semaforów bramek
    for(int i = 0; i < 4; i++) {
        if (sem_init(&platform->lower_gates[i], 1, 1) != 0) {
            perror("Failed to initialize gate semaphore");
            // Cleanup previously initialized semaphores
            sem_destroy(&platform->platform_capacity);
            for(int j = 0; j < i; j++) {
                sem_destroy(&platform->lower_gates[j]);
            }
            free(platform);
            return NULL;
        }
    }
    
    platform->lower_platform_count = 0;
    platform->upper_platform_count = 0;
    
    return platform;
}

int enter_lower_platform(Platform* platform, int skier_id, int is_vip) {
    int current_value;
    sem_getvalue(&platform->platform_capacity, &current_value);
    printf("Skier %d (VIP %d) trying to enter. Current platform capacity: %d\n", skier_id, is_vip, current_value);
    
    if (sem_wait(&platform->platform_capacity) == 0) {
        int gate = is_vip ? 0 :randomInt(1,3);
  
        
        if (sem_wait(&platform->lower_gates[gate]) == 0) {
            pthread_mutex_lock(&platform->count_mutex);
            platform->lower_platform_count++;
            int current_count = platform->lower_platform_count;
            pthread_mutex_unlock(&platform->count_mutex);
            printf("Skier %d successfully entered through gate %d. Total on platform: %d\n", 
                   skier_id, gate, current_count);
            sem_post(&platform->lower_gates[gate]);
            return 0;
        }
        sem_post(&platform->platform_capacity);
    }
    
    printf("Skier %d failed to enter platform\n", skier_id);
    return -1;
}

void exit_lower_platform(Platform* platform) {
    pthread_mutex_lock(&platform->count_mutex);
    platform->lower_platform_count--;
    int current_count = platform->lower_platform_count;
    pthread_mutex_unlock(&platform->count_mutex);
    sem_post(&platform->platform_capacity);
    printf("A skier left the platform. Total remaining: %d\n", current_count);
}

void cleanup_platform(Platform* platform) {
    if (platform != NULL) {
        sem_destroy(&platform->platform_capacity);
        
        for(int i = 0; i < 4; i++) {
            sem_destroy(&platform->lower_gates[i]);
        }
        
        pthread_mutex_destroy(&platform->count_mutex);
        free(platform);
    }
}