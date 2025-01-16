#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "platform.h"

Platform* init_platform() {
    Platform* platform = malloc(sizeof(Platform));
    
    // Inicjalizacja semafora pojemności peronu
    sem_init(&platform->platform_capacity, 1, MAX_PLATFORM_CAPACITY);
    
    // Inicjalizacja semaforów bramek dolnego peronu
    for(int i = 0; i < 4; i++) {
        sem_init(&platform->lower_gates[i], 1, 1);
    }
    
    // Inicjalizacja semaforów wyjść górnego peronu
    for(int i = 0; i < 2; i++) {
        sem_init(&platform->upper_exits[i], 1, 1);
    }
    
    platform->lower_platform_count = 0;
    platform->upper_platform_count = 0;
    
    return platform;
}

// ... (reszta implementacji pozostaje bez zmian)

int enter_lower_platform(Platform* platform, int skier_id) {
    printf("Attempting to enter lower platform - skier %d\n", skier_id);
    
    // Sprawdź czy jest miejsce na peronie
    if (sem_trywait(&platform->platform_capacity) == -1) {
        printf("Platform is full - skier %d waiting\n", skier_id);
        return -1; // Peron jest pełny
    }
    
    // Wybierz losową bramkę
    int gate = rand() % 4;
    printf("Skier %d trying gate %d\n", skier_id, gate);
    
    // Próbuj przejść przez bramkę
    if (sem_wait(&platform->lower_gates[gate]) == 0) {
        platform->lower_platform_count++;
        printf("Skier %d entered lower platform through gate %d. Platform count: %d\n", 
               skier_id, gate, platform->lower_platform_count);
        sem_post(&platform->lower_gates[gate]);
        return 0;
    }
    
    // Jeśli nie udało się przejść przez bramkę, zwolnij miejsce na peronie
    printf("Failed to enter through gate - skier %d\n", skier_id);
    sem_post(&platform->platform_capacity);
    return -1;
}

void exit_lower_platform(Platform* platform) {
    platform->lower_platform_count--;
    sem_post(&platform->platform_capacity);
    printf("Skier left lower platform. Platform count: %d\n", 
           platform->lower_platform_count);
}

void enter_upper_platform(Platform* platform) {
    platform->upper_platform_count++;
    printf("Skier arrived at upper platform. Platform count: %d\n", 
           platform->upper_platform_count);
}

int exit_upper_platform(Platform* platform, int exit_number, int skier_id) {
    if (exit_number < 0 || exit_number >= 2) {
        return -1;
    }
    
    // Próbuj wyjść wybraną drogą
    if (sem_wait(&platform->upper_exits[exit_number]) == 0) {
        platform->upper_platform_count--;
        printf("Skier %d left upper platform through exit %d. Platform count: %d\n", 
               skier_id, exit_number, platform->upper_platform_count);
        sem_post(&platform->upper_exits[exit_number]);
        return 0;
    }
    
    return -1;
}

void cleanup_platform(Platform* platform) {
    // Zniszczenie wszystkich semaforów
    sem_destroy(&platform->platform_capacity);
    
    for(int i = 0; i < 4; i++) {
        sem_destroy(&platform->lower_gates[i]);
    }
    
    for(int i = 0; i < 2; i++) {
        sem_destroy(&platform->upper_exits[i]);
    }
    
    free(platform);
}