#ifndef WORKER_H
#define WORKER_H

#include "platform.h"
#include "shared_memory.h"

typedef struct {
    int id;
    Platform* platform;
    volatile int is_running;
    volatile int emergency_stop;
} Worker;

// Inicjalizacja pracownika
Worker* init_worker(int id, Platform* platform);

// Funkcje dla procesów pracowników
void run_worker1(Worker* worker);  // Obsługa dolnej stacji
void run_worker2(Worker* worker);  // Obsługa górnej stacji

// Zatrzymanie awaryjne
void emergency_stop_platform(Worker* worker);

// Wznowienie pracy po zatrzymaniu awaryjnym
void resume_platform(Worker* worker);

// Zwolnienie zasobów pracownika
void cleanup_worker(Worker* worker);

#endif