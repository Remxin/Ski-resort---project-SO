#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "worker.h"
#include "utils.h"

Worker* init_worker(int id, Platform* platform) {
    Worker* worker = malloc(sizeof(Worker));
    worker->id = id;
    worker->platform = platform;
    worker->is_running = 1;
    worker->emergency_stop = 0;
    return worker;
}

void run_worker1(Worker* worker) {
    printf("Worker 1 started managing lower platform\n");
    
    while(worker->is_running) {
        // Sprawdzanie warunków bezpieczeństwa na dolnym peronie
        if(worker->platform->lower_platform_count > MAX_PLATFORM_CAPACITY) {
            printf("Worker 1: Emergency stop - platform overcrowded!\n");
            emergency_stop_platform(worker);
        }
        
        // Symulacja pracy
        sleep(1);
    }
}

void run_worker2(Worker* worker) {
    printf("Worker 2 started managing upper platform\n");
    
    while(worker->is_running) {
        // Sprawdzanie warunków bezpieczeństwa na górnym peronie
        if(worker->platform->upper_platform_count > MAX_PLATFORM_CAPACITY) {
            printf("Worker 2: Emergency stop - platform overcrowded!\n");
            emergency_stop_platform(worker);
        }
        
        // Symulacja pracy
        sleep(1);
    }
}

void emergency_stop_platform(Worker* worker) {
    worker->emergency_stop = 1;
    printf("Worker %d: Platform emergency stopped\n", worker->id);
    
    // Symulacja czasu potrzebnego na rozwiązanie problemu
    sleep(randomInt(3, 6));
    
    // Automatyczne wznowienie po rozwiązaniu problemu
    resume_platform(worker);
}

void resume_platform(Worker* worker) {
    worker->emergency_stop = 0;
    printf("Worker %d: Platform operation resumed\n", worker->id);
}

void cleanup_worker(Worker* worker) {
    worker->is_running = 0;
    free(worker);
}