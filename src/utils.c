#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>    // dla msgget
#include <sys/types.h>  // dla key_t
#include <sys/ipc.h>    // dla IPC_CREAT
#include "utils.h"
#include "config.h"

struct CheckoutQueues init_queues() {
    struct CheckoutQueues queues;
    queues.queue1_id = msgget(QUEUE_KEY_1, IPC_CREAT | 0666);
    queues.queue2_id = msgget(QUEUE_KEY_2, IPC_CREAT | 0666);
    return queues;
}

struct CheckoutQueues get_queues() {
   struct CheckoutQueues queues;
   queues.queue1_id = msgget(QUEUE_KEY_1, 0666);
   queues.queue2_id = msgget(QUEUE_KEY_2, 0666);
   return queues;
}

int randomInt(int min, int max) {
     if (min >= max) {
        // Obsługuje przypadek, gdy min >= max
        fprintf(stderr, "Min should be less than max\n");
        return -1; // Zwracamy błąd lub inne wartości w zależności od potrzeb
    }
    return rand() % (max - min) + min;
}

// Funkcja zwraca true lub false na podstawie zadanego prawdopodobieństwa
bool random_chance(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        fprintf(stderr, "Probability must be in range [0, 1].\n");
        return false;
    }

    // Wygeneruj losową liczbę z przedziału [0, 1)
    double rand_value = (double)rand() / (double)RAND_MAX;
    
    // Jeśli wygenerowana liczba jest mniejsza od prawdopodobieństwa, zwróć true
    return rand_value < probability;
}