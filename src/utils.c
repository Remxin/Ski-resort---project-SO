#include "utils.h"
#include <stdlib.h>
#include <stdio.h>


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