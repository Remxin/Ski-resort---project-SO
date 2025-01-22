#include "lift.h"

Lift *init_lift() {
    Lift* lift = malloc(sizeof(Lift));

    sem_init(&lift->lower_exit, 1, 1);
    sem_init(&lift->upper_exit, 1, 1);
    sem_init(&lift->seats_mutex, 1, 1);
    sem_init(&lift->vip_queue, 1, 0);
    sem_init(&lift->regular_queue, 1, 0);

    for (int i = 0; i < SEATS_COUNT; i++) {
        for (int j = 0; j < SEAT_PLACES; j++) {
            lift->seats[i][j] = -1;
        }
    }

    lift->enter_pointer = 0;
    lift->exit_pointer = SEATS_COUNT / 2 + 1;

    return lift;
}

int find_family_space(Lift* lift, int family_size) {
    int free_spaces = 0;
    for(int j = 0; j < SEAT_PLACES; j++) {
        if(lift->seats[lift->enter_pointer][j] == -1) {
            free_spaces++;
        }
    }
    return free_spaces >= family_size;
}



void exit_lift(Lift *lift) {
    sem_wait(&lift->seats_mutex);

    for (int j = 0; j < SEAT_PLACES; j++) {
        if (lift->seats[lift->exit_pointer][j] != -1) {
            printf("Skier %d exited lift from seat %d\n", 
            lift->seats[lift->exit_pointer][j], lift->exit_pointer);
            lift->seats[lift->exit_pointer][j] = -1;
        }
    }

    sem_post(&lift->seats_mutex);
}

void *operate_lift(void *arg) {
    Lift* lift = (Lift*)arg;
    while(1) {
        sleep(LIFT_TRAVEL_TIME);
        sem_wait(&lift->seats_mutex);
        exit_lift(lift);

        lift->exit_pointer = (lift->exit_pointer + 1) % SEATS_COUNT;
        lift->enter_pointer = (lift->enter_pointer + 1) % SEATS_COUNT;

        sem_post(&lift->vip_queue);
        sleep(1); // VIP can now enter lift
        sem_post(&lift->regular_queue);

        sem_post(&lift->seats_mutex);
    }
    return NULL;
}