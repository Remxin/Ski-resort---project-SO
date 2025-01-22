#ifndef LIFT_H
#define LIFT_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "config.h"

// typedef struct {
//     sem_t platform_capacity;     // kontrola liczby osób na peronie
//     sem_t lower_gates[4];       // 4 bramki wejściowe na dolnym peronie
//     sem_t upper_exits[2];       // 2 drogi wyjściowe z górnego peronu
//     int lower_platform_count;    // licznik osób na dolnym peronie
//     int upper_platform_count;    // licznik osób na górnym peronie
// } Platform;

typedef struct {
    sem_t lower_exit;
    sem_t upper_exit;
    sem_t seats_mutex;      // do synchronizacji dostępu do miejsc
    sem_t vip_queue;        // do priorytetyzacji VIPów
    sem_t regular_queue;    // dla zwykłych narciarzy
    int seats[SEATS_COUNT][SEAT_PLACES];
    int enter_pointer;
    int exit_pointer;
} Lift;

Lift* init_lift();

int find_family_space(Lift* lift, int family_size);
void exit_lift(Lift *lift);
void *operate_lift(void *arg);



#endif