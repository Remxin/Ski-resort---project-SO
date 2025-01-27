#ifndef PLATFORM_H
#define PLATFORM_H

#include <semaphore.h>
#include "config.h"

typedef struct {
    sem_t platform_capacity;     // kontrola liczby osób na peronie
    sem_t lower_gates[4];       // 4 bramki wejściowe na dolnym peronie
    sem_t upper_exits[2];       // 2 drogi wyjściowe z górnego peronu
    int lower_platform_count;    // licznik osób na dolnym peronie
    int upper_platform_count;    // licznik osób na górnym peronie
} Platform;

// Usuwamy shared_data z inicjalizacji
Platform* init_platform();

// Próba wejścia na dolny peron przez bramkę
int enter_lower_platform(Platform* platform, int skier_id, int is_vip, int num_child);

// Wyjście z dolnego peronu (wjazd na górę)
void exit_lower_platform(Platform* platform, int num_child);

// Wejście na górny peron (przyjazd z dołu)
void enter_upper_platform(Platform* platform);

// Wyjście z górnego peronu przez wybraną drogę
int exit_upper_platform(Platform* platform, int exit_number, int skier_id);

// Zamknięcie i zwolnienie zasobów peronu
void cleanup_platform(Platform* platform);

#endif