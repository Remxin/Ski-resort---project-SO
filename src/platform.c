#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "platform.h"
#include "utils.h"

Platform* init_platform() {
    Platform* platform = malloc(sizeof(Platform));
    if (platform == NULL) {
        perror("Failed to allocate platform");
        return NULL;
    }
    
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

int enter_lower_platform(Platform* platform, int skier_id, int is_vip, int num_child) {
    int current_value;
    sem_getvalue(&platform->platform_capacity, &current_value);
    printf("Skier %d trying to enter. Current platform capacity: %d\n", skier_id, current_value);

    // Sprawdź miejsce dla rodzica i dzieci
    if (sem_wait(&platform->platform_capacity) == -1) {
        perror("sem_wait platform_capacity failed");
        return -1;
    }
    
    // Jeśli są dzieci, zajmij dla nich miejsca
    for(int i = 0; i < num_child; i++) {
        if (sem_wait(&platform->platform_capacity) == -1) {
            // W przypadku błędu, zwolnij zajęte wcześniej miejsca
            for(int j = 0; j <= i; j++) {
                sem_post(&platform->platform_capacity);
            }
            perror("sem_wait platform_capacity failed for children");
            return -1;
        }
    }

    // Wybór bramki: 0 dla VIP, losowa 1-3 dla zwykłych
    int gate = is_vip ? 0 : randomInt(1, 3);
    
    if (sem_wait(&platform->lower_gates[gate]) == -1) {
        // W przypadku błędu na bramce, zwolnij wszystkie zajęte miejsca
        for(int i = 0; i < (1 + num_child); i++) {
            sem_post(&platform->platform_capacity);
        }
        perror("sem_wait lower_gates failed");
        return -1;
    }

    // Aktualizacja licznika osób
    platform->lower_platform_count += (1 + num_child);
    printf("Skier %d successfully entered through gate %d with %d children. Total on platform: %d\n",
           skier_id, gate, num_child, platform->lower_platform_count);
    
    sem_post(&platform->lower_gates[gate]);
    return 0;
}

void exit_lower_platform(Platform* platform, int num_child) {
    // Zmniejsz licznik o rodzica i dzieci
    platform->lower_platform_count -= (1 + num_child);
    
    // Zwolnij miejsce dla rodzica i każdego dziecka
    for(int i = 0; i < (1 + num_child); i++) {
        sem_post(&platform->platform_capacity);
    }
    
    printf("A skier with %d children left the platform. Total remaining: %d\n", 
           num_child, platform->lower_platform_count);
}

void cleanup_platform(Platform* platform) {
    if (platform != NULL) {
        sem_destroy(&platform->platform_capacity);
        
        for(int i = 0; i < 4; i++) {
            sem_destroy(&platform->lower_gates[i]);
        }
        
        free(platform);
    }
}