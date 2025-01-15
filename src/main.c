#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "skier.h"
#include "config.h"



void* skier_thread(void* arg) {
    Skier* skier = create_skier();
    if (skier == NULL) {
        perror("Failed to create skier");
        return NULL;
    }

    free(skier); // Free allocated memory for skier
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t threads[MAX_SKIERS];

    // Create and start skier processes
    for (int i = 0; i < MAX_SKIERS; i++) {
        if (pthread_create(&threads[i], NULL, skier_thread, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Wait for all skier threads to finish
    for (int i = 0; i < MAX_SKIERS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
