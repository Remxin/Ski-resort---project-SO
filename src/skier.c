#include "skier.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void *child_process(void *arg) {
    int* child_age = (int*)arg;
    printf("Child process: Child age is %d\n", *child_age);

    free(child_age);  // Zwolnienie pamięci po użyciu
    return NULL;
}

// Funkcja tworząca narciarza oraz jego dzieci
Skier* create_skier() {
    int age = randomInt(CHILD_AGE, MAX_AGE);  // Generowanie losowego wieku
    bool vip = random_chance(VIP_CHANCE);            // Randomly assign VIP status
    int children_count = (age >= 18 && random_chance(HAS_CHILDREN_PROBABILITY)) ? rand() % MAX_CHILDREN + 1 : 0; // Random children count

    Skier* skier = malloc(sizeof(Skier));
    if (!skier) {
        perror("Failed to allocate memory for skier");
        return NULL;
    }
    skier->id = rand();
    skier->age = age;
    skier->isVIP = vip;
    skier->children_count = children_count;

    printf("Creating skier with ID: %d, Age: %d, VIP: %d, children: %d\n", skier->id, skier->age, skier->isVIP, skier->children_count);

    skier->children = (pthread_t*)malloc(sizeof(pthread_t) * skier->children_count);
    if (skier->children == NULL) {
        perror("Failed to allocate memory for children threads");
        free(skier);  // Zwolnij pamięć dla narciarza, jeśli alokacja nie powiodła się
        return NULL;
    }

    // Tworzenie dzieci
    for (int i = 0; i < skier->children_count; i++) {
        int* child_age = malloc(sizeof(int));
        if (!child_age) {
            perror("Failed to allocate memory for child age");
            free(skier->children);  // Zwolnij pamięć dla dzieci
            free(skier);  // Zwolnij pamięć dla narciarza
            return NULL;
        }
        *child_age = randomInt(YOUNG_CHILD_AGE, CHILD_AGE);  // Losowanie wieku dziecka
        pthread_create(&skier->children[i], NULL, child_process, (void*)child_age);
    }

    // Czekaj na zakończenie wątków dzieci
    for (int i = 0; i < skier->children_count; i++) {
        pthread_join(skier->children[i], NULL);
    }

    free(skier->children);  // Zwolnij pamięć dla tablicy wątków dzieci
    return skier;  // Zwróć utworzonego narciarza
}
