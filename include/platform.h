#ifndef PLATFORM_H
#define PLATFORM_H

#include <semaphore.h>
#include <pthread.h>
#include "config.h"
#include "utils.h"

typedef struct {
   sem_t platform_capacity;
   sem_t lower_gates[4];
   sem_t* counter_sem;
   int lower_platform_count;
   int upper_platform_count;
} Platform;

// Usuwamy shared_data z inicjalizacji
Platform* init_platform();

// Próba wejścia na dolny peron przez bramkę
int enter_lower_platform(Platform* platform, int skier_id, int is_vip);

// Wyjście z dolnego peronu (wjazd na górę)
void exit_lower_platform(Platform* platform);

// Wejście na górny peron (przyjazd z dołu)
void enter_upper_platform(Platform* platform);

// Wyjście z górnego peronu przez wybraną drogę
int exit_upper_platform(Platform* platform, int exit_number, int skier_id);

// Zamknięcie i zwolnienie zasobów peronu
void cleanup_platform(Platform* platform);

#endif